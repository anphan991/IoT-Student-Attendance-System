import os
from datetime import datetime, timedelta, timezone
from typing import Optional

# Định nghĩa múi giờ Việt Nam (UTC+7)
VN_TZ = timezone(timedelta(hours=7))

# Thư viện Database
from supabase import create_client, Client

# Thư viện Web & Bảo mật
from fastapi.middleware.cors import CORSMiddleware
from fastapi import FastAPI, Depends, HTTPException, status
from fastapi.security import OAuth2PasswordBearer, OAuth2PasswordRequestForm
from fastapi.responses import FileResponse
from pydantic import BaseModel
from passlib.context import CryptContext
from jose import JWTError, jwt 
import pandas as pd
from dotenv import load_dotenv

print("🚀 SERVER ĐANG KHỞI ĐỘNG...", flush=True)

# Load biến môi trường
load_dotenv()

# ========== CẤU HÌNH BẢO MẬT ==========
SECRET_KEY = os.getenv("JWT_SECRET", "chuoi_bi_mat_sieu_kho_doan_123456") 
ALGORITHM = "HS256"
ACCESS_TOKEN_EXPIRE_MINUTES = 30 

class AttendanceUpdate(BaseModel):
    mssv: str
    ma_lop: str
    action: str # "increase" hoặc "decrease"

# Model nhận dữ liệu quét thẻ từ ESP32
class ScanData(BaseModel):
    uid: str
    device_id: str
    time_scan: int

# ========== CẤU HÌNH SUPABASE ==========
SUPABASE_URL = os.getenv("SUPABASE_URL")
SUPABASE_KEY = os.getenv("SUPABASE_KEY")
supabase: Client = create_client(SUPABASE_URL, SUPABASE_KEY)

# ========== KHỞI TẠO FASTAPI ==========
app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"], 
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

pwd_context = CryptContext(schemes=["bcrypt"], deprecated="auto")
oauth2_scheme = OAuth2PasswordBearer(tokenUrl="token")

# ========== MODELS & AUTH HELPER ==========
class Token(BaseModel):
    access_token: str
    token_type: str

class TokenData(BaseModel):
    username: Optional[str] = None

def verify_password(plain_password, hashed_password):
    return pwd_context.verify(plain_password, hashed_password)

def create_access_token(data: dict, expires_delta: Optional[timedelta] = None):
    to_encode = data.copy()
    expire = datetime.utcnow() + (expires_delta if expires_delta else timedelta(minutes=15))
    to_encode.update({"exp": expire})
    return jwt.encode(to_encode, SECRET_KEY, algorithm=ALGORITHM)

async def get_current_user(token: str = Depends(oauth2_scheme)):
    credentials_exception = HTTPException(
        status_code=status.HTTP_401_UNAUTHORIZED,
        detail="Token không hợp lệ",
        headers={"WWW-Authenticate": "Bearer"},
    )
    try:
        payload = jwt.decode(token, SECRET_KEY, algorithms=[ALGORITHM])
        username: str = payload.get("sub")
        if username is None:
            raise credentials_exception
        token_data = TokenData(username=username)
    except JWTError:
        raise credentials_exception
    return token_data.username


# ========== API NHẬN ĐIỂM DANH TỪ ESP32 ==========

@app.post("/api/attendance")
def receive_attendance_from_esp32(data: ScanData):
    uid = data.uid
    device_id = data.device_id
    time_scan = data.time_scan

    if not uid or not device_id: 
        raise HTTPException(status_code=400, detail="Thiếu thông tin")

    try:
        # 1. Lấy thông tin phòng học từ thiết bị
        res_device = supabase.table("thiet_bi").select("phong_hoc").eq("device_id", device_id).execute()
        if not res_device.data or not res_device.data[0].get("phong_hoc"):
            return {"status": "error", "message": f"Thiết bị chưa được gắn phòng."}
        
        phong_hoc_hien_tai = res_device.data[0]["phong_hoc"]

        # 2. XÁC ĐỊNH LỚP HỌC DỰA VÀO GIỜ QUẸT THẺ (Ép về giờ VN)
        scan_dt = datetime.fromtimestamp(time_scan, tz=timezone.utc).astimezone(VN_TZ)
        scan_time_str = scan_dt.strftime('%H:%M:%S')

        res_lop = supabase.table("lop_hoc") \
            .select("ma_lop") \
            .eq("phong_hoc", phong_hoc_hien_tai) \
            .lte("gio_bat_dau", scan_time_str) \
            .gte("gio_ket_thuc", scan_time_str) \
            .execute()

        if not res_lop.data:
            return {"status": "error", "message": "Hiện tại phòng này không có ca học nào."}
            
        ma_lop_hien_tai = res_lop.data[0]["ma_lop"]

        # 3. Tiến hành điểm danh cho mssv vào lớp ma_lop_hien_tai
        # Lấy MSSV từ UID
        res_sv = supabase.table("sinh_vien").select("mssv, ho_ten").eq("uid", uid).execute()
        if not res_sv.data: return {"status": "error", "message": "Thẻ chưa đăng ký."}
        mssv = res_sv.data[0]["mssv"]
        ho_ten = res_sv.data[0]["ho_ten"]

        # Cập nhật số buổi đi học (Giao lại cho Trigger lo số vắng)
        res_ds = supabase.table("danh_sach_lop").select("id, so_buoi_di_hoc").eq("mssv", mssv).eq("ma_lop", ma_lop_hien_tai).execute()
        
        status_log = "Guest"
        if res_ds.data:
            current_id = res_ds.data[0]["id"]
            so_buoi_cu = res_ds.data[0].get("so_buoi_di_hoc", 0)
            
            supabase.table("danh_sach_lop").update({"so_buoi_di_hoc": so_buoi_cu + 1}).eq("id", current_id).execute()
            status_log = "Present"
            print(f"✔ Điểm danh thành công: {ho_ten} - Lớp {ma_lop_hien_tai}")

        # Ghi Log
        thoi_gian = scan_dt.isoformat()
        supabase.table("nhat_ky_diem_danh").insert({
            "mssv": mssv, "device_id": device_id, "thoi_gian": thoi_gian, "status": status_log
        }).execute()

        return {"status": "success", "message": "Đã ghi nhận"}

    except Exception as e:
        print(f"❌ Lỗi: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# ========== API LẤY DANH SÁCH SINH VIÊN CHO ESP32 ==========
@app.get("/api/students/{device_id}")
def get_students_for_device(device_id: str):
    try:
        # 1. Tìm phòng của máy này
        res_device = supabase.table("thiet_bi").select("phong_hoc").eq("device_id", device_id).execute()
        if not res_device.data: return []
        phong_hoc = res_device.data[0]["phong_hoc"]

        # 2. Tìm lớp đang diễn ra ngay lúc ESP32 khởi động/gọi API
        # Ép dùng giờ Việt Nam để tra cứu lớp
        now_time_str = datetime.now(VN_TZ).strftime('%H:%M:%S')
        res_lop = supabase.table("lop_hoc") \
            .select("ma_lop") \
            .eq("phong_hoc", phong_hoc) \
            .lte("gio_bat_dau", now_time_str) \
            .gte("gio_ket_thuc", now_time_str) \
            .execute()

        if not res_lop.data:
            # Nếu đang ở giờ nghỉ, trả về rỗng. ESP32 sẽ hiển thị "Chưa có lớp"
            return []
            
        ma_lop = res_lop.data[0]["ma_lop"]

        # 3. Trả về danh sách sinh viên của lớp đó
        res_students = supabase.table("danh_sach_lop") \
            .select("mssv, sinh_vien(uid, ho_ten)") \
            .eq("ma_lop", ma_lop) \
            .execute()

        danh_sach = []
        for row in res_students.data:
            sv_info = row.get("sinh_vien")
            if sv_info and sv_info.get("uid"):
                danh_sach.append({
                    "uid": sv_info["uid"],
                    "mssv": row["mssv"],
                    "name": sv_info["ho_ten"]
                })
        return danh_sach
    except Exception as e:
        print(f"Lỗi API get_students: {e}")
        return []


# ========== API XÁC THỰC ==========
@app.post("/token", response_model=Token)
async def login_for_access_token(form_data: OAuth2PasswordRequestForm = Depends()):
    response = supabase.table("admin_users").select("*").eq("username", form_data.username).execute()
    user = response.data
    if not user or not verify_password(form_data.password, user[0]['password_hash']):
        raise HTTPException(status_code=401, detail="Sai tài khoản/mật khẩu")
    
    return {"access_token": create_access_token(data={"sub": form_data.username}), "token_type": "bearer"}


# ========== API THỐNG KÊ & APP WEB ==========
@app.get("/")
async def read_index():
    return FileResponse("index.html")

@app.get("/my-classes")
def get_my_classes(current_user: str = Depends(get_current_user)):
    try:
        return supabase.table("lop_hoc").select("ma_lop, ten_lop").eq("username_giang_vien", current_user).execute().data
    except Exception as e:
        return {"error": str(e)}

@app.get("/stats")
def stats(ma_lop: str, current_user: str = Depends(get_current_user)):
    try:
        response = supabase.table("thong_ke_lop_hoc") \
            .select("*") \
            .eq("username_giang_vien", current_user) \
            .eq("ma_lop", ma_lop) \
            .execute()
        
        data = response.data
        return {
            "danh_sach_chi_tiet": data
        }
    except Exception as e:
        return {"error": str(e)}

@app.get("/export")
def export(ma_lop: str, current_user: str = Depends(get_current_user)):
    try:
        data = supabase.table("thong_ke_lop_hoc").select("*").eq("username_giang_vien", current_user).eq("ma_lop", ma_lop).execute().data
        df = pd.DataFrame(data)
        path = f"bao_cao_{ma_lop}.xlsx"
        df.to_excel(path, index=False)
        return FileResponse(path=path, filename=path)
    except Exception as e:
        return {"error": str(e)}


# ========== API CẬP NHẬT THỦ CÔNG ==========
@app.post("/update-attendance")
def update_attendance_manual(data: AttendanceUpdate, current_user: str = Depends(get_current_user)):
    try:
        check = supabase.table("lop_hoc").select("username_giang_vien").eq("ma_lop", data.ma_lop).execute()
        if not check.data or check.data[0]["username_giang_vien"] != current_user:
            raise HTTPException(status_code=403, detail="Không có quyền!")

        sv = supabase.table("danh_sach_lop").select("so_buoi_di_hoc").eq("mssv", data.mssv).eq("ma_lop", data.ma_lop).execute()
        if not sv.data: raise HTTPException(404, "Không tìm thấy SV")
        
        current_val = sv.data[0]["so_buoi_di_hoc"]
        new_val = current_val + 1 if data.action == "increase" else current_val - 1
        
        # 1. Update số buổi đi học (Cột Vắng vẫn do Database tự tính)
        supabase.table("danh_sach_lop") \
            .update({"so_buoi_di_hoc": new_val}) \
            .eq("mssv", data.mssv) \
            .eq("ma_lop", data.ma_lop) \
            .execute()

        # 2. TỰ TAY GHI LOG (Thay vì nhờ Database ghi)
        supabase.table("nhat_ky_diem_danh").insert({
            "mssv": data.mssv,
            "device_id": "WEB_MANUAL", 
            "thoi_gian": datetime.now(VN_TZ).isoformat(),
            "status": f"Manual Update: {current_val} -> {new_val}"
        }).execute()

        return {"msg": "OK", "new_val": new_val}
    except Exception as e:
        raise HTTPException(status_code=400, detail=str(e))
# Sử dụng Python phiên bản 3.12 (khớp với môi trường local của bạn)
FROM python:3.12-slim

# Thiết lập thư mục làm việc trong container
WORKDIR /code

# Copy file requirements.txt trước để tận dụng cơ chế cache của Docker
COPY ./requirements.txt /code/requirements.txt

# Cài đặt các thư viện cần thiết
RUN pip install --no-cache-dir --upgrade -r /code/requirements.txt

# Copy toàn bộ mã nguồn vào container
# Docker sẽ tự động bỏ qua các file trong .dockerignore (xem bước 2)
COPY . .

# Hugging Face Spaces yêu cầu ứng dụng chạy trên cổng 7860
# Chạy FastAPI bằng Uvicorn
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "7860"]
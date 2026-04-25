# I. Mục tiêu xây dựng thành phần xử lý dữ liệu

## 1. Giải thích thành phần hệ thống

- Hệ thống gồm các thành phần và tương tác như sau  
- ESP32 -> HiveMQ Broker -> Backend -> Frontend  

- Mỗi thành phần lớn lại có các thành phần logic và thành phần vật lý nhỏ  

    + ví dụ ESP32 có thành phần phần cứng là 3 cảm biến và vi điều khiển  
    + và thành phần logic là 5 task: 3 task sensor, 1 cụm task WiFi Manager, 1 cụm task MQTT Connection  

    + HiveMQ Broker thì chịu :v vì đây là hệ thống thứ ba  

    + Frontend thì chỉ có 2 thành phần logic là giao diện (HTML + CSS) và thành phần xử lý logic hiển thị  

    + Riêng backend ta sẽ chia thành 2 thành phần nhỏ hơn và sẽ được mô tả sau đây  

---

## 2. Chi tiết hóa mức 1 thành phần backend backend

- Backend có nhiệm vụ thu nhập dữ liệu từ HiveMQ, xử lý và đẩy lên web client  

- Do đó ta nên tách Backend làm 2 thành phần là (1) và (2)  

    + (1) - hệ thống thu nhập (1.1) và xử lý dữ liệu (1.2)  
    + (1.1) - Data Ingestion = Server thu nhập dữ liệu và gửi vào (1.2)  
    + (1.2) - Processing = Thành phần xử lý dữ liệu  

    + nói là (1.1) gửi vào (1.2) nghĩa là gọi hàm xử lý dữ liệu thôi  

    + (1.1 + 1.2 = 1) là 1 chương trình duy nhất viết bằng python  

    + thành phần (2) là server cung cấp API cho web client  

---

- Thành phần (1.1)/Data Ingestion ta sẽ code để đọc dữ liệu từ HiveMQ, sau đó gửi dữ liệu xuống  
thành phần (1.2) để nó xử lý dữ liệu, sau khi xử lý xong thì lưu vào file .csv  

---

- Thành phần (2) sẽ được viết bằng javascrip  

    + nhiệm vụ của nó là tạo kết nối tới web client bằng giao thức WebSocket  
    + và cung cấp API để web client lấy dữ liệu  

---

## 3. Chi tiết hóa mức 2 các thành phần backend

- Thành phần (1). Dữ liệu thu được từ HiveMQ về Server python sẽ được Server xử lý và lưu vào file .csv  
                ví dụ: ECG_HEART_RATE.csv, ECG_WARNING.csv, FLEX_BREATHING_RATE.csv, FLEX_WARNING.csv  

- Thành phần (2).  

    + (2) ko cần biết (1) đã gửi dữ liệu xuống file .csv chưa, vì đây là 2 thành phần độc lập  

    + do đó (2) cứ có dữ liệu mới là nó sẽ gửi cho webclient dữ liệu mới nhất  

    + Với thiết kế này (1), (2) là 2 bên đọc lập, giống kiểu 1 producer và 1 consumer  

    + ưu điểm là tách biệt logic, dễ debug và mở rộng  

    + nhược điểm là chấp nhận chậm 1 chút, ko sao cả  

---

# II. Thực hiện

## 1.

Anh sẽ nghỉ 2-3 hôm để xử lý bài tập của các môn khác  

---

## 2.

sau khi nghỉ xong thì ah sẽ sửa lại folder Backend/ trên github  

- cụ thể ah sẽ tạo 2 folder riêng cho 2 chương trình py và javascrip  
- ah sẽ code sẵn cái khung  

- còn chi tiết 1 số đoạn do các em làm thì các em ko nên viết trực tiếp vào các file này mà  
  sẽ tạo riêng các file (copy) để viết nào, trong có trình code có thể các e sẽ cần tự tạo các file riêng  
  tự tổ chức folder riêng, tự tạo và đặt lên file .csv riêng (các tên file .csv ở trên là ah ví dụ thôi)  

- sau khi code xong ah sẽ ghép vào làm 1  
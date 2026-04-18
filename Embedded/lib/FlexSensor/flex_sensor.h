#ifndef AD8232_TASKS_H
#define AD8232_TASKS_H

//1. khai báo các thư viện cần để sử dụng
#include <Arduino.h>

//2. Khai báo các biến, struct của header này
// ví dụ 1 struct đại diện cho cảm biến này
// hoặc mảng lưu trữ tín hiệu, tùy thiết kế

//3. khai báo (nhớ là chỉ khai báo thôi) các hàm tự định nghĩa
void TaskFlexSenSor(void *pvParameters); // hàm xử lý logic chính
void initFlexTask(void *pvParameters); // hàm khởi tạo tasks
// nên cần thêm các hàm khác nữa ví dụ các hàm in ra màn hình terminal dữ liệu gì đó
// hay các hàm phụ để tính toán logic

// lưu ý
// tên hàm, biến, struct nên đặt rõ nghĩa, ko chung chung khi x, y, z 
//chừ các biến cục bộ trong hàm


#endif
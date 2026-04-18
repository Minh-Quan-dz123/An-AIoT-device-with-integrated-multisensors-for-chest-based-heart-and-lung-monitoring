//1 include thư viện/header (include thư viện ở file .h và .cpp có ý nghĩa khác nhau)
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "flex_sensor.h"
// thêm nếu cần,....


// 2 define các giá trị cần thiết, setup giá trị cho các biến ban đầu

// 3 code các hàm
// 3.1. hàm xử lý tín hiệu cho ECG
// đây là hàm chính được hệ điều hành quản lý, hãy viết logic chính
void TaskFlexSenSor(*pvParameters) 
{
  while (1) 
  {
    // TODO: bạn tự xử lý logic ở đây
  }
}

// 3.2. hàm wrapper để tạo task (ko cần sửa sang gì đâu nha, hoặc thích thì sửa)
void initFlexTask(void *pvParameters) 
{
  xTaskCreate(
    TaskFlexSenSor,   // hàm task sẽ chạy
    "Flex Task",     // tên task (chỉ để debug)
    8192,           // kích thước stack (byte)
    NULL,           // tham số truyền vào task
    2,              // độ ưu tiên (priority)
    NULL            // handle của task (nếu cần quản lý)
    );
}

// 3.3 các hàm khác phụ để phục vụ cho hàm TaskECG
// tự đặt tên, tự dùng như code c++ bình thường

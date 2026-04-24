#include "mqtt_buffer.h"

// 1 constructor
MQTTBuffer::MQTTBuffer()
{
    queue = NULL;
}

// 2 begin
bool MQTTBuffer::begin(int maxSize)
{
    // tạo queue
    queue = xQueueCreate(maxSize, sizeof(MQTTMessage));
    return queue != NULL;
}

// 3 push
bool MQTTBuffer::push(const MQTTMessage &msg)
{
    // 3.1 nếu null thì thôi
    if (queue == NULL) return false;

    // 3.2 non-blocking send, đẩy lên queue
    return xQueueSend(queue, &msg, 0) == pdTRUE;
}

// 4 pop - lấy ra từ queue
bool MQTTBuffer::pop(MQTTMessage &msg)
{
    if (queue == NULL) return false;

    // non-blocking receive
    return xQueueReceive(queue, &msg, 0) == pdTRUE;
}

bool MQTTBuffer::isEmpty()
{
    if (queue == NULL) return true;

    // kiểm tra rỗng ko
    return uxQueueMessagesWaiting(queue) == 0;
}
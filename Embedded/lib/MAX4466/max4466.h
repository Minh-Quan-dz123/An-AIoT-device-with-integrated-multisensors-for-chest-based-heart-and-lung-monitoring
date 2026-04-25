#ifndef MIC_MAX4466_H
#define MIC_MAX4466_H

#include <Arduino.h>

#define MAX4466_PIN 35

// khởi tạo task đọc mic
void startMicTask();
void micTask(void *parameter);

#endif
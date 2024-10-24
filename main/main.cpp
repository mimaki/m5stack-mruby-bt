#include <M5Stack.h>

#include <stdio.h>
#include <stdint.h>
#include "mruby.h"
#include "mruby/dump.h"
#include "mruby/variable.h"
#include "mruby/string.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

/* Target application */
#define APP_MRB "/autorun.mrb"

extern "C" int mirb(mrb_state*);

extern "C" void InitGattServer();

void mrubyTask(void *pvParameters)
{
  printf("Hello, m5stack-mruby-bt!\n");
  while (1) {
    micros();
  }
}


extern "C" void app_main()
{
  InitGattServer();

  initArduino();
  xTaskCreatePinnedToCore(mrubyTask, "mrubyTask", 8192, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
}

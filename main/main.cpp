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
  mrb_state *mrb = (mrb_state*)pvParameters;

  printf("mrb=%p\n", mrb);
  printf("Hello, m5stack-mruby-bt!\n");

  /* Initialize M5Stack */
  M5.begin();

  /* Open mruby VM */
  // mrb = mrb_open();
  if (!mrb) {
    M5.lcd.print("mrb_open() failed.\n");
    goto ERROR;
  }
  printf("mruby VM opened. mrb=%p\n", mrb);

  mrb_close(mrb);
  printf("mruby VM closed.\n");

ERROR:
  while (1) {
    micros();
  }
}


#define MRUBY_TASK_STACK_SIZE 16384

extern "C" void app_main()
{
  /* Start GATT Server */
  InitGattServer();

  /* Initialize Arduino component */
  initArduino();

  /* Initialize M5Stack component */
  M5.begin();

  /* Initalize mruby */
  mrb_state *mrb = mrb_open();
  printf("mrb=%p\n", mrb);

  /* Start mruby task */
  xTaskCreatePinnedToCore(mrubyTask, "mrubyTask", MRUBY_TASK_STACK_SIZE, mrb, 1, NULL, ARDUINO_RUNNING_CORE);
}

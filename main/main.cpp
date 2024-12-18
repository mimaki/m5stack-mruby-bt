#include <M5Stack.h>

#include <stdio.h>
#include <stdint.h>
#define MRB_NO_BOXING
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

static mrb_value
run_app_mrb(mrb_state *mrb, const char *fname)
{
  mrb_value val = mrb_nil_value();
  size_t size;
  uint8_t *buf;

  /* open mrb file */
  fs::File file = SD.open(fname, FILE_READ);
  if (!file) {
    // M5.lcd.printf("(%s) load error.\n", fname);
    return val;
  }

  /* load mrb file */
  size = file.size();
  buf = new uint8_t[size];
  file.read(buf, size);
  /* launch application */
  val = mrb_load_irep(mrb, buf);
  /* free buffer and close mrb file */
  delete [] buf;
  file.close();

  return val;
}

void mrubyTask(void *pvParameters)
{
  mrb_state *mrb = (mrb_state*)pvParameters;
  mrb_value val;

  // printf("mrb=%p\n", mrb);
  // printf("Hello, m5stack-mruby-bt!\n");

  /* Initialize M5Stack */
  M5.begin();
  puts("M5Stack component initialized.");

  /* Open mruby VM */
  // mrb = mrb_open();
  // if (!mrb) {
  //   M5.lcd.print("mrb_open() failed.\n");
  //   goto ERROR;
  // }
  // printf("mruby VM opened. mrb=%p\n", mrb);

  if (M5.BtnA.isReleased()) {
    val = run_app_mrb(mrb, APP_MRB);
    if (mrb->exc) {
      val = mrb_funcall(mrb, mrb_obj_value(mrb->exc), "inspect", 0);
      mrb->exc = 0;
    }
    else {
      if (!mrb_nil_p(val)) {
        M5.lcd.print(" => ");
        if (!mrb_string_p(val)) {
          val = mrb_obj_as_string(mrb, val);
        }
      }
    }
    if (!mrb_nil_p(val)) {
      M5.lcd.printf("%s\n", mrb_str_to_cstr(mrb, val));
    }
  }

  /* Enter interactive mode */
  mirb(mrb);

  mrb_close(mrb);
  printf("mruby VM closed.\n");

// ERROR:
  while (1) {
    micros();
  }
}


#define MRUBY_TASK_STACK_SIZE 8192

// #undef ARDUINO_RUNNING_CORE
// #define ARDUINO_RUNNING_CORE  0

extern "C" void GattNotify(uint8_t*, size_t);
extern "C" mrb_value mrb_ble_notify(mrb_state *mrb, mrb_value self)
{
  char *s;
  mrb_int len;
  mrb_get_args(mrb, "s", &s, &len);
  GattNotify((uint8_t*)s, (size_t)len);
  return self;
}

extern "C" mrb_value mrb_sound_tone(mrb_state *mrb, mrb_value self)
{
  mrb_int freq, msec;
  mrb_get_args(mrb, "ii", &freq, &msec);
  M5.Speaker.begin();
  M5.Speaker.tone(freq, msec);
  delay(msec);
  M5.Speaker.end();
  return self;
}

extern "C" void app_main()
{
  /* Start GATT Server */
  InitGattServer();
  puts("GATT Server initialized.");

  /* Initialize Arduino component */
  initArduino();
  puts("Arduino component initialized.");

  /* Initialize M5Stack component */
  // M5.begin();

  /* Initalize mruby */
  mrb_state *mrb = mrb_open();
  if (!mrb) {
    M5.lcd.print("mrb_open() failed.\n");
    return;
  }
  puts("mruby VM initialized.");
  mrb_define_method(mrb, mrb->object_class, "ble_notify", mrb_ble_notify, MRB_ARGS_REQ(1));
  struct RClass *sound = mrb_define_class(mrb, "Sound", mrb->object_class);
  mrb_define_method(mrb, sound, "tone", mrb_sound_tone, MRB_ARGS_REQ(2));

  /* Start mruby task */
  xTaskCreatePinnedToCore(mrubyTask, "mrubyTask", MRUBY_TASK_STACK_SIZE, mrb, 1, NULL, ARDUINO_RUNNING_CORE);
  // mrubyTask(mrb);
}

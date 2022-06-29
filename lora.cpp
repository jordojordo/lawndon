#include "lora.h"

void Lora::get_datetime(unsigned long *date, unsigned long *time) {
  if (date)
    *date = _date;
  if (time)
    *time = _time;
}
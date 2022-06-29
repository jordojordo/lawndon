#define LORA_H

class Lora {
public:
  void get_datetime(unsigned long *date, unsigned long *time);

private:
  unsigned long _date, _time;
};
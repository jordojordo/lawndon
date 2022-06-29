#define TIME_H

struct timehm_t {
  byte hour;
  byte minute;
};

typedef struct timehm_t timehm_t;

struct date_t {
  byte dayOfWeek;
  byte day;
  byte month;
  short year;
};

typedef struct date_t date_t;

struct datetime_t {
  timehm_t time;
  date_t date;
};

typedef struct datetime_t datetime_t;

struct timer_t {
  boolean active;
  timehm_t startTime;
  timehm_t stopTime;
  byte daysOfWeek;
};

typedef struct timer_t timer_t;
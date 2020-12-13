#include "ds1307.h"
#include "esphome/core/log.h"
#include <sys/time.h>
#include "errno.h"

namespace esphome
{
  namespace ds1307
  {

    static const char *TAG = "ds1307";

    void DS1307Component::setup()
    {
      ESP_LOGCONFIG(TAG, "Setting up DS1307...");
      Wire.begin();
      if (!read_data())
      {
        this->mark_failed();
      }
    }

    bool DS1307Component::read_data()
    {
      if (!this->read_bytes(0, this->ds1307_.raw, sizeof(this->ds1307_.raw)))
      {
        ESP_LOGE(TAG, "Can't read I2C data.");
        return false;
      }

      if (ds1307_.reg.ch)
      {
        ESP_LOGW(TAG, "Clock halted.");
        return false;
      }

      struct tm t
      {
        0
      };
      t.tm_year = ds1307_.reg.year + 10 * ds1307_.reg.year_10 + 100;
      t.tm_mon = ds1307_.reg.month + 10 * ds1307_.reg.month_10 - 1;
      t.tm_mday = ds1307_.reg.day + 10 * ds1307_.reg.day_10;
      t.tm_hour = ds1307_.reg.hour + 10 * ds1307_.reg.hour_10;
      t.tm_min = ds1307_.reg.minute + 10 * ds1307_.reg.minute_10;
      t.tm_sec = ds1307_.reg.second + 10 * ds1307_.reg.second_10;

      time_t tt = mktime(&t);

      struct timeval timev
      {
        .tv_sec = static_cast<time_t>(tt), .tv_usec = 0,
      };
      ESP_LOGVV(TAG, "Got epoch %u", tt);
      timezone tz = {0, 0};
      int ret = settimeofday(&timev, &tz);
      if (ret == EINVAL)
      {
        // Some ESP8266 frameworks abort when timezone parameter is not NULL
        // while ESP32 expects it not to be NULL
        ret = settimeofday(&timev, nullptr);
      }

      if (ret != 0)
      {
        ESP_LOGW(TAG, "setimeofday() failed with code %d", ret);
      }

      auto time = this->now();
      char buf[128];
      time.strftime(buf, sizeof(buf), "%c");
      ESP_LOGD(TAG, "Synchronized system time from hardware clock: %s", buf);
      ESP_LOGV(TAG, "Rate select: %0d", ds1307_.reg.rs);
      ESP_LOGV(TAG, "Square-Wave Enable (SQWE): %0d", ds1307_.reg.sqwe);
      ESP_LOGV(TAG, "Output Control (OUT): %0d", ds1307_.reg.out);
      return true;
    }

    bool DS1307Component::write_data()
    {
      if (!this->read_bytes(0, this->ds1307_.raw, sizeof(this->ds1307_.raw)))
      {
        ESP_LOGE(TAG, "Can't read I2C data.");
        return false;
      }
      auto time = this->now();
      if (!time.is_valid())
      {
        ESP_LOGE(TAG, "System time is not valid.");
        return false;
      }
      ds1307_.reg.ch = 0;
      ds1307_.reg.year = time.year % 10;
      ds1307_.reg.year_10 = time.year / 10 % 10;
      ds1307_.reg.month = time.month % 10;
      ds1307_.reg.month_10 = time.month / 10;
      ds1307_.reg.day = time.day_of_month % 10;
      ds1307_.reg.day_10 = time.day_of_month / 10;
      ds1307_.reg.hour = time.hour % 10;
      ds1307_.reg.hour_10 = time.hour / 10;
      ds1307_.reg.minute = time.minute % 10;
      ds1307_.reg.minute_10 = time.minute / 10;
      ds1307_.reg.second = time.second % 10;
      ds1307_.reg.second_10 = time.second / 10;
      ds1307_.reg.weekday = time.day_of_week;
      if (!this->write_bytes(0, this->ds1307_.raw, sizeof(this->ds1307_.raw)))
      {
        ESP_LOGE(TAG, "Can't write I2C data.");
        return false;
      }

      char buf[128];
      time.strftime(buf, sizeof(buf), "%c");
      ESP_LOGD(TAG, "Synchronized system time to hardware clock: %s", buf);
      return true;
    }

  } // namespace ds1307
} // namespace esphome

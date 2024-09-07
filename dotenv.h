#ifndef DOTENV_H
#define DOTENV_H

#include <Arduino.h>
#include ".env.h"

class DotEnv
{
public:
  static bool load()
  {
    return true;
  }

  static String get(const char *key)
  {
    if (strcmp(key, "WIFI_SSID") == 0)
    {
      return String(WIFI_SSID);
    }
    else if (strcmp(key, "WIFI_PASS") == 0)
    {
      return String(WIFI_PASS);
    }
    return String();
  }
};

#endif // DOTENV_H
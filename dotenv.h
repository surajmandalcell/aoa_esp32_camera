#ifndef DOTENV_H
#define DOTENV_H

#include <Arduino.h>
#include <SPIFFS.h>

class DotEnv
{
public:
  static bool load(const char *filename = "/.env")
  {
    if (!SPIFFS.begin(true))
    {
      Serial.println("An error occurred while mounting SPIFFS");
      return false;
    }

    File file = SPIFFS.open(filename, "r");
    if (!file)
    {
      Serial.println("Failed to open .env file");
      return false;
    }

    while (file.available())
    {
      String line = file.readStringUntil('\n');
      line.trim();

      // Skip comments and empty lines
      if (line.startsWith("#") || line.length() == 0)
      {
        continue;
      }

      int separatorPos = line.indexOf('=');
      if (separatorPos != -1)
      {
        String key = line.substring(0, separatorPos);
        String value = line.substring(separatorPos + 1);

        // Remove quotes if present
        if ((value.startsWith("\"") && value.endsWith("\"")) ||
            (value.startsWith("'") && value.endsWith("'")))
        {
          value = value.substring(1, value.length() - 1);
        }

        key.trim();
        value.trim();

        if (key.length() > 0 && value.length() > 0)
        {
          setenv(key.c_str(), value.c_str(), 1);
        }
      }
    }

    file.close();
    return true;
  }

  static String get(const char *key)
  {
    const char *value = getenv(key);
    return value ? String(value) : String();
  }
};

#endif // DOTENV_H
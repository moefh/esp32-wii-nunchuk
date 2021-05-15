# esp32-wii-nunchuk

This is a library to use the Wii Nunchuk with the ESP32 via I2C.  It
can be used both with the Arduino IDE or with code using the ESP-IDF
directly.  To use the library in your Arduino IDE sketch, just copy
the files `wii_i2c.c` and `wii_i2c.h` to your sketch directory.

This library uses the [ESP-IDF I2C
API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html)
because the [Arduino Wire
library](https://www.arduino.cc/en/reference/wire) doesn't work
reliably in the ESP32 with Wii controllers.

![ESP32 connected to a Wii Nunchuk](images/photo.jpg)

This library supports the **Wii Nunchuk** and the **Wii Classic
Controller**.  It shouldn't be hard to adapt it to work with other I2C
devices that plug in the Wiimote (like the Classic Controller Pro, Wii
Motion Plus, etc.) with the information available in the [Wiibrew
project](http://wiibrew.org/wiki/Wiimote/Extension_Controllers), but I
have none of these devices so I don't know for sure.

### Coexisting with Arduino Wire Library

The Arduino Wire Library for the ESP32 (from `Wire.h`) uses I2C port 0
for the `Wire` object, and port 1 for the `Wire1` object.  So don't
use `Wire` if using I2C port 0 with this library, and don't use
`Wire1` if using I2C port 1.

### Example Code

Here's a simple example using the Wii Nunchuk. For a more complete
example that detects and handles multiple controller types, see
`esp32-wii-nunchuk.ino`.

```C++
#include "wii_i2c.h"

// pins connected to the Nunchuk:
#define PIN_SDA 32
#define PIN_SCL 33

// ESP32 I2C port (0 or 1):
#define WII_I2C_PORT 0

void setup()
{
  Serial.begin(115200);

  if (wii_i2c_init(WII_I2C_PORT, PIN_SDA, PIN_SCL) != 0) {
    Serial.printf("Error initializing nunchuk :(");
    return;
  }
  wii_i2c_request_state();
}

void loop()
{
  const unsigned char *data = wii_i2c_read_state();
  wii_i2c_request_state();
  if (! data) {
    Serial.printf("no data available :(")
  } else {
    wii_i2c_nunchuk_state state;
    wii_i2c_decode_nunchuk(data, &state);
    Serial.printf("Stick position: (%d,%d)\n", state.x, state.y);
    Serial.printf("C button is %s\n", (state.c) ? "pressed" : "not pressed");
    Serial.printf("Z button is %s\n", (state.z) ? "pressed" : "not pressed");
  }
  delay(1000);
}
```

### Multi-Core Support

If you have time-sensitive code that can't wait for the controller to
respond, use the API function that spawns a task that reads the
controller state in a different core.  For example:

```C++
#include "wii_i2c.h"

// pins connected to the controller:
#define PIN_SDA 32
#define PIN_SCL 33

// ESP32 I2C port (0 or 1):
#define WII_I2C_PORT 0

// CPU id where the task will run (1=the core
// where your code usually runs, 0=the other core):
#define READ_TASK_CPU 0

// delay in milliseconds between controller reads:
#define READ_DELAY 30

static unsigned int controller_type;

void setup()
{
  Serial.begin(115200);

  if (wii_i2c_init(WII_I2C_PORT, PIN_SDA, PIN_SCL) != 0) {
    Serial.printf("Error initializing nunchuk :(");
    return;
  }

  // if you want to read the controller identity,
  // do it BEFORE starting the read task:
  const unsigned char *ident = wii_i2c_read_ident();
  controller_type = wii_i2c_decode_ident(ident);

  // start the a task that reads the controller state in a different CPU:
  if (wii_i2c_start_read_task(READ_TASK_CPU, READ_DELAY) != 0) {
    Serial.printf("Error creating task to read controller state");
    return;
  }
}

void loop()
{
  // this function always returns quickly, either
  // with new data or NULL if data isn't ready:
  const unsigned char *data = wii_i2c_read_data_from_task();
  if (data) {
     // decode data according to controller_type:
     //   wii_i2c_decode_nunchuk(data, &nunchuk_state);
     //   wii_i2c_decode_classic(data, &classic_state);
  }
  
  // do other timing-sensitive stuff
}
```

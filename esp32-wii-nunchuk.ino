#include "wii_i2c.h"

#define PIN_SDA  32
#define PIN_SCL  33
#define WII_I2C_PORT 0

unsigned int controller_type = 0;

void show_nunchuk(const unsigned char *data)
{
  wii_i2c_nunchuk_state state;
  wii_i2c_decode_nunchuk(data, &state);
        
  Serial.printf("a = (%5d,%5d,%5d)\n", state.acc_x, state.acc_y, state.acc_z);
  Serial.printf("d = (%5d,%5d)\n", state.x, state.y);
  Serial.printf("c=%d, z=%d\n", state.c, state.z);
}

void show_classic(const unsigned char *data)
{
  wii_i2c_classic_state state;
  wii_i2c_decode_classic(data, &state);

  Serial.printf("lx,ly = (%3d,%3d)\n", state.lx, state.ly);
  Serial.printf("rx,ry = (%3d,%3d)\n", state.rx, state.ry);
  Serial.printf("a lt,rt = (%3d,%3d)\n", state.a_lt, state.a_rt);
  Serial.printf("d lt,rt = (%d,%d)\n", state.d_lt, state.d_rt);
  Serial.printf("a,b,x,y = (%d,%d,%d,%d)\n", state.a, state.b, state.x, state.y);
  Serial.printf("up, down, left, right = (%d,%d,%d,%d)\n", state.up, state.down, state.left, state.right);
  Serial.printf("home, plus, minus = (%d,%d,%d)\n", state.home, state.plus, state.minus);
  Serial.printf("zl, zr = (%d,%d)\n", state.zl, state.zr);
}

void setup()
{
  Serial.begin(115200);
  Serial.printf("Starting...\n");

  if (wii_i2c_init(WII_I2C_PORT, PIN_SDA, PIN_SCL) != 0) {
    Serial.printf("ERROR initializing wii i2c controller\n");
    return;
  }
  const unsigned char *ident = wii_i2c_read_ident();
  if (! ident) {
    Serial.printf("no ident :(\n");
    return;
  }
  
  controller_type = wii_i2c_decode_ident(ident);
  switch (controller_type) {
  case WII_I2C_IDENT_NUNCHUK: Serial.printf("-> nunchuk detected\n"); break;
  case WII_I2C_IDENT_CLASSIC: Serial.printf("-> classic controller detected\n"); break;
  default:                    Serial.printf("-> unknown controller detected: 0x%06x\n", controller_type); break;
  }
  wii_i2c_request_state();
}

void loop()
{
  const unsigned char *data = wii_i2c_read_state();
  wii_i2c_request_state();
  if (data) {
    switch (controller_type) {
    case WII_I2C_IDENT_NUNCHUK: show_nunchuk(data); break;
    case WII_I2C_IDENT_CLASSIC: show_classic(data); break;
    default:
      Serial.printf("data: %02x %02x %02x %02x %02x %02x\n",
                    data[0], data[1], data[2], data[3], data[4], data[5]);
      break;
    }
  } else {
    Serial.printf("no data :(\n");
  }
  
  delay(1000);
}

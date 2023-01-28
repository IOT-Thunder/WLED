# 1 "/var/folders/tq/dj3_2fcj26qby33c4y80pgnx_xp4d8/T/tmphw1cg52q"
#include <Arduino.h>
# 1 "/Users/hitesh11.kumar/Documents/GitHub/WLED/wled00/wled00.ino"
# 13 "/Users/hitesh11.kumar/Documents/GitHub/WLED/wled00/wled00.ino"
#include "wled.h"
void setup();
void loop();
#line 15 "/Users/hitesh11.kumar/Documents/GitHub/WLED/wled00/wled00.ino"
void setup() {
  WLED::instance().setup();
}

void loop() {
  WLED::instance().loop();
}
#include "time.h"
#include "data.h"

namespace TIME {
  uint32_t uptime;

  void loop(const uint32_t currMillis) {
    static uint32_t uptimeUpdateTime = 0;
    // раз в секунду увеличиваем uptime
    if (currMillis > uptimeUpdateTime + 1000 || currMillis < uptimeUpdateTime) {
      uptime++;
      uptimeUpdateTime += 1000; 
    }
  }
}

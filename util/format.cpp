#include <fmt/format.h>
#include <stdio.h>
#include <time.h>

int main(int, char**) {
  time_t rawtime = 1648575216;
  struct tm ts;
  ts = *gmtime(&rawtime);
  fmt::print("{}/{}/{:04d}/{:02d}/{:02d}/{:02d}-{:02d}.bin\n", 1, "M1", ts.tm_year + 1900, ts.tm_mon, ts.tm_mday, ts.tm_hour, ts.tm_hour + 1);
}
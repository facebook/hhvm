#include "hphp/runtime/server/server-worker.h"
#include "hphp/util/timer.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// ServerJob

ServerJob::ServerJob() {
  Timer::GetMonotonicTime(start);
}

void ServerJob::stopTimer(const struct timespec &reqStart) {
  if (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats) {
    // This measures [enqueue:dequeue]
    timespec end;
    Timer::GetMonotonicTime(end);
    time_t dsec = end.tv_sec - start.tv_sec;
    long dnsec = end.tv_nsec - start.tv_nsec;
    int64_t dusec = dsec * 1000000 + dnsec / 1000;
    ServerStats::Log("page.wall.queuing", dusec);

    // This measures [request start:dequeue]
    dsec = start.tv_sec - reqStart.tv_sec;
    dnsec = start.tv_nsec - reqStart.tv_nsec;
    dusec = dsec * 1000000 + dnsec / 1000;
    ServerStats::Log("page.wall.request_read_time", dusec);
  }
}

}

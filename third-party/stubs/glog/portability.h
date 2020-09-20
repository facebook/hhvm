#ifndef HPHP_THIRD_PARTY_GLOG_PORTABILITY
#define HPHP_THIRD_PARTY_GLOG_PORTABILITY

#include <glog/logging.h>

#define CHECK_NULL(val) CHECK_EQ((val), static_cast<void*>(NULL))

#endif // #ifndef HPHP_THIRD_PARTY_GLOG_PORTABILITY


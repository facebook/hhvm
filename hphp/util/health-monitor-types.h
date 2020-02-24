/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_HOST_HEALTH_MONITOR_TYPES_H_
#define incl_HPHP_HOST_HEALTH_MONITOR_TYPES_H_

#include <string>

#include <folly/Likely.h>

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

/*
 * The enum type for describing various system health level.  The
 * system health monitor will expose one of the listed health level.
 *
 * Beware if you change the order or the numerical values, as we
 * assume smaller numbers indicating better health condition here.
 */
enum class HealthLevel {
  Bold = 0,
  Moderate = 1,
  Cautious = 2,
  NoMore = 3,
  BackOff = 4,
  NumLevels = 5                  // insert new values before NumLevels
};

/*
 * The interface of specific system metrics used by the system health
 * monitor.
 */
struct IHealthMonitorMetric {
  virtual ~IHealthMonitorMetric() {}
  virtual HealthLevel evaluate() = 0;
  virtual bool enabled() = 0;
  void registerSelf();           // register the metric in the monitor
};

/*
 * The interface for observers which subscribe the output of the
 * system health monitor.
 */
struct IHostHealthObserver {
  virtual ~IHostHealthObserver() {}
  virtual void notifyNewStatus(HealthLevel newStatus) = 0;
  virtual HealthLevel getHealthLevel() = 0;
};

/*
 * Helper function to convert HealthLevel enum value to integer number that is
 * bigger for better health condition.
 */
inline int64_t healthLevelToInt(HealthLevel level) {
  constexpr int32_t kMaxHealth = 100;
  // Smaller HealthLevel indicates better health condition, under
  // which this function returns a bigger number.
  static_assert(static_cast<int>(HealthLevel::Bold) == 0, "");
  constexpr int32_t kMaxLevel =
    static_cast<int32_t>(HealthLevel::NumLevels) - 1;
  if (LIKELY(level == HealthLevel::Bold)) {
    return kMaxHealth;
  }
  auto const result = kMaxHealth *
    (kMaxLevel - static_cast<int32_t>(level)) / kMaxLevel;
  return result;
}

////////////////////////////////////////////////////////////////////////////////
}

#endif

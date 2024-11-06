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

#include "hphp/runtime/server/accounting.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/util/hardware-counter.h"
#include "hphp/util/service-data.h"


namespace HPHP {

void Accounting::onRequestEnd(Transport *transport) {
  static auto instructionsTimeSeries = ServiceData::createTimeSeries(
    "accounting.instructions",
    {
      // For convenient global monitoring
      ServiceData::StatsType::SUM, 
      // For per-request monitoring and accounting
      ServiceData::StatsType::AVG, 
      // For per-server MIPS
      ServiceData::StatsType::RATE
    },
    {std::chrono::seconds(60)}
  );

  int64_t elapsedInstructions = 
    HardwareCounter::GetInstructionCount() - transport->getInstructions();
  instructionsTimeSeries->addValue(elapsedInstructions);

  static auto requestsTimeSeries = ServiceData::createTimeSeries(
    "accounting.requests",
    {
      // For accounting
      ServiceData::StatsType::SUM,
      // For monitoring
      ServiceData::StatsType::RATE 
    },
    {std::chrono::seconds(60)}
  );

  requestsTimeSeries->addValue(1);
}

} // namespace HPHP

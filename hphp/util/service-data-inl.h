/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace ServiceData {

namespace detail {

inline std::chrono::seconds nowAsSeconds() {
  auto now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::seconds>(
    now.time_since_epoch());
}

} // namespace detail

inline void ExportedTimeSeries::addValue(int64_t value) {
  m_timeseries->addValue(detail::nowAsSeconds(), value);
}

inline void ExportedTimeSeries::addValue(int64_t value, int64_t times) {
  m_timeseries->addValue(detail::nowAsSeconds(), value, times);
}

inline void ExportedTimeSeries::addValueAggregated(int64_t sum,
                                                   int64_t nsamples) {
  m_timeseries->addValueAggregated(detail::nowAsSeconds(), sum, nsamples);
}

inline void ExportedHistogram::addValue(int64_t value) {
  m_histogram->addValue(value);
}

inline void ExportedHistogram::removeValue(int64_t value) {
  m_histogram->removeValue(value);
}

}  // namespace ServiceData

//////////////////////////////////////////////////////////////////////

}  // namespace HPHP

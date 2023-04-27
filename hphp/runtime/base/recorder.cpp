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

#include "hphp/runtime/base/recorder.h"

#include <folly/Likely.h>
#include <folly/Random.h>

#include "hphp/runtime/base/runtime-option.h"

namespace HPHP {

  Recorder::Recorder() : m_enabled{false}, m_path{} {}

  bool Recorder::enabled() const {
    return m_enabled;
  }

  void Recorder::onSessionExit() {
    m_enabled = false;
    m_path = "";
  }

  void Recorder::onSessionInit() {
    if (UNLIKELY(RuntimeOption::EvalRecordReplay && RuntimeOption::EvalRecordSampleRate > 0)) {
      m_enabled = folly::Random::rand64(RuntimeOption::EvalRecordSampleRate) == 0;
      if (m_enabled) {
        m_path = RuntimeOption::EvalRecordDir + '/' + std::to_string(folly::Random::rand64());
      }
    }
  }

} // namespace HPHP

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

#include "hphp/runtime/vm/jit/bc-marker.h"

#include <folly/Format.h>
#include <folly/Conv.h>

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

std::string BCMarker::show() const {
  assertx(valid());
  return folly::format(
    "--- bc {}{}{}{}, fp {}, spOff {} ({}){}",
    m_sk.offset(),
    resumeModeShortName(m_sk.resumeMode()),
    m_sk.hasThis()  ? "t" : "",
    m_sk.prologue() ? "p" : "",
    m_fp ? folly::to<std::string>(m_fp->id()) : "_",
    m_spOff.offset,
    m_sk.func()->fullName(),
    m_profTransID != kInvalidTransID
      ? folly::format(" [profTrans={}]", m_profTransID).str()
      : std::string{}
  ).str();
}

bool BCMarker::valid() const {
  if (isDummy()) return true;
  // Note, we can't check stack bounds here because of inlining, and
  // instructions like idx, which can create php-level calls.
  return
    m_sk.valid() &&
    m_sk.offset() >= m_sk.func()->base() &&
    m_sk.offset() < m_sk.func()->past();
}

//////////////////////////////////////////////////////////////////////

}}

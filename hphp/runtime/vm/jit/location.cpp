/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/location.h"

#include "hphp/util/assertions.h"

#include <folly/Format.h>
#include <folly/Hash.h>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

std::string show(Location loc) {
  switch (loc.tag()) {
    case LTag::Local:
      return folly::format("Local{{{}}}", loc.localId()).str();
    case LTag::Stack:
      return folly::format("Stack{{{}}}", loc.stackIdx().offset).str();
  }
  not_reached();
}

size_t Location::Hash::operator()(Location loc) const {
  return folly::hash::hash_combine(
    static_cast<uint32_t>(loc.m_tag),
    loc.m_local.locId
  );
}

///////////////////////////////////////////////////////////////////////////////

}}

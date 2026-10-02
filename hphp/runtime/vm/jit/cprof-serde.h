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

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include <folly/Range.h>

#include "hphp/runtime/vm/jit/cprof-key.h"
#include "hphp/runtime/vm/jit/cprof-record.h"

namespace HPHP::jit::cprof {

std::optional<std::vector<uint8_t>>
serializeContProfFuncKey(const ContProfFuncKey&);

std::optional<ContProfFuncKey> deserializeContProfFuncKey(folly::ByteRange);

std::optional<std::vector<uint8_t>>
serializeContProfProfileRecord(const ContProfProfileRecord&);

std::optional<ContProfProfileRecord>
deserializeContProfProfileRecord(folly::ByteRange);

}

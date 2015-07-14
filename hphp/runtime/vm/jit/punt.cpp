/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/punt.h"

#include <folly/Format.h>

#include "hphp/runtime/vm/func.h"

namespace HPHP { namespace jit {
//////////////////////////////////////////////////////////////////////

FailedIRGen::FailedIRGen(const char* _file, int _line, const char* _func)
  : std::runtime_error(folly::format("FailedIRGen @ {}:{} in {}",
                                     _file, _line, _func).str())
  , file(_file)
  , line(_line)
  , func(_func)
{}

FailedTraceGen::FailedTraceGen(const char* file, int line, const char* why)
  : std::runtime_error(folly::format("FailedTraceGen @ {}:{} - {}",
                                     file, line, why).str())
{}

FailedCodeGen::FailedCodeGen(const char* _file,
                             int _line,
                             const char* _func,
                             uint32_t _bcOff,
                             const Func* _vmFunc,
                             bool _resumed,
                             TransID _profTransId)
  : std::runtime_error(
      folly::format("FailedCodeGen @ {}:{} in {}. {}@{}{}. tid = {}",
                    _file, _line, _func,
                    _vmFunc->fullName(), _bcOff,
                    _resumed ? "r" : "", _profTransId).str())
  , file(_file)
  , line(_line)
  , func(_func)
  , bcOff(_bcOff)
  , vmFunc(_vmFunc)
  , resumed(_resumed)
  , profTransId(_profTransId)
{}

//////////////////////////////////////////////////////////////////////
}}

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


#ifndef incl_HPHP_VM_PROFILER_H_
#define incl_HPHP_VM_PROFILER_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP {

void profileOneArgument(TypedValue value, int param, const Func* func);
void logType(const Func* func, const char* typeString, int64_t param);
void writeProfileInformationToDisk();
const char* giveTypeString(const TypedValue* value);
std::string dumpRawParamInfo(const Func* function);
void initFuncTypeProfileData(const Func* func);
}

#endif

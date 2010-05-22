/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __EVAL_BASE_H__
#define __EVAL_BASE_H__

#include <util/base.h>
#include <runtime/base/base_includes.h>
#include <runtime/base/program_functions.h>
#include <runtime/eval/base/ast_ptr.h>
#include <runtime/eval/runtime/eval_frame_injection.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class VariableEnvironment;

struct Location {
  const char *file;
  int line1;
  int char1;
  String toString() const;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_BASE_H__ */

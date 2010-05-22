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
#ifndef __EVAL_STRICT_MODE_H__
#define __EVAL_STRICT_MODE_H__

#include <runtime/base/base_includes.h>
#include <runtime/base/util/exceptions.h>

/**
 * This file works in concert with the StrictLevel and StrictFatal variables
 * defined in runtime_option.h. It implements the policy regarding
 * the "strict" checking mode of hphpi. Do we throw an exception when 
 * a semantic check failed, or just a warning ? Or do we do anything at all ?
 */

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class StrictMode {
public:
  enum StrictLevel {
    StrictNone = 0,

    StrictBasic = 1,
    StrictHardCore = 2,
  };
};

void throw_strict(const ExtendedException &exn,
                  int strict_level_exn);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EVAL_STRICT_MODE_H__

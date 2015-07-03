/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_SYSTEM_PROFILER_H_
#define incl_HPHP_SYSTEM_PROFILER_H_

#include <string>

#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/ext/hotprofiler/ext_hotprofiler.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// classes

/*
 * Events interesting to a system profiler.
 *
 * A system profiler should instantiate one instance of this class,
 * and bind the global g_system_profiler to point to that instance.
 * That binding can be done in a dynamic shared object.
 */

class SystemProfiler {
public:
  SystemProfiler() { }
  virtual ~SystemProfiler() { }

  /*
   * Called when a PHP execution error is handled.
   */
  virtual void errorCallBack(const ExtendedException &ee,
                             int errnum,
                             const std::string &msg) = 0;
  /*
   * Called when PHP loads a file.
   */
  virtual void fileLoadCallBack(const std::string &name) = 0;

  /*
   * Called once to get the hotprofiler.
   */
  virtual Profiler *getHotProfiler() = 0;
};

extern SystemProfiler *g_system_profiler;

}

#endif // incl_HPHP_SYSTEM_PROFILER_H_

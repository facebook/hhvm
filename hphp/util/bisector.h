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
#ifndef incl_HPHP_UTIL_BISECTOR_H_
#define incl_HPHP_UTIL_BISECTOR_H_

#include "hphp/util/compilation-flags.h"
#include "hphp/util/trace.h"

namespace HPHP {

/*
  Use this class to help bisect issues with optimizations.

  Declare a bisector via:

    BISECTOR(name);

  Then guard each optimization by

    if (name.go(<optional msg>)) {
      // do the optimization
    }

  Then run via eg

    env TRACE=bisector:1 name2=1000000 <program>

  and look in the trace output to see how many times the optimization happened.
  eg:

   name : 0
   name : 1
   name : 2

  Now you can bisect down to the optimization thats causing issues by setting
  name1 and name2 (name1 will default to 0 if not set). If name1 < name2 it
  will run the optimizations in the interval [name1, name2). If name1 > name2
  it will skip the optimizations in the interval [name2, name1).

  Note that its usually best to at least start by bisecting with name2 only,
  since (for some optimizations) skipping earlier optimizations can change
  later results.
*/

struct BisectorHelper {
  TRACE_SET_MOD(bisector)

  explicit BisectorHelper(const char* e) : env(e) {
    if (debug) {
      auto const p1 = getenv((env + std::string("1")).c_str());
      auto const p2 = getenv((env + std::string("2")).c_str());
      if (p2) {
        low = p1 ? atoi(p1) : 0;
        high = atoi(p2);
      }
    }
  }
  bool go(const char* msg = nullptr) {
    if (!debug || (!high && !low)) return true;
    if ((count++ - low) < (high - low)) {
      FTRACE(1, "{} : {} {}\n",
             env, count - 1, msg ? msg : "");
      return true;
    }
    return false;
  }
private:
  const char *env;
  uint32_t low = 0u;
  uint32_t high = 0u;
  uint32_t count = 0u;
};

#define BISECTOR(name) static BisectorHelper name(#name);

}

#endif

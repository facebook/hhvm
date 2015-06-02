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

/*
 * TODO(#3468751): delete this header.
 */

#ifndef incl_HPHP_BASE_H_
#define incl_HPHP_BASE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <cinttypes>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/poll.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <stack>
#include <string>
#include <map>
#include <list>
#include <set>
#include <memory>
#include <deque>
#include <exception>
#include <functional>

#include <boost/interprocess/sync/interprocess_upgradable_mutex.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/filesystem/operations.hpp>

#include "hphp/util/compilation-flags.h"
#include "hphp/util/hash.h"
#include "hphp/util/assertions.h"
#include "hphp/util/functional.h"

#ifdef __INTEL_COMPILER
#define va_copy __builtin_va_copy
#endif

namespace HPHP {
  using std::string;
  using std::vector;
  using std::dynamic_pointer_cast;
  using std::static_pointer_cast;
}

#endif

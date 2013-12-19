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
#include "hphp/util/abi-cxx.h"

#include <memory>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <string>
#include <algorithm>
#include <mutex>

#include <cxxabi.h>
#include <execinfo.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

typedef std::lock_guard<std::mutex> G;
std::mutex nameCacheLock;
std::map<void*,std::string> nameCache;

}

//////////////////////////////////////////////////////////////////////

char* getNativeFunctionName(void* codeAddr) {
  {
    G g(nameCacheLock);
    auto it = nameCache.find(codeAddr);
    if (it != end(nameCache)) {
      auto ret = new char[it->second.size() + 1];
      std::copy(it->second.data(),
                it->second.data() + it->second.size() + 1,
                ret);
      return ret;
    }
  }

  void* buf[1] = {codeAddr};
  char** symbols = backtrace_symbols(buf, 1);
  char* functionName = nullptr;
  if (symbols != nullptr) {
    //
    // the output from backtrace_symbols looks like this:
    // ../path/hhvm/hhvm(_ZN4HPHP2VM6Transl17interpOneIterInitEv+0) [0x17cebe9]
    //
    // we first want to extract the mangled name from it to get this:
    // _ZN4HPHP2VM6Transl17interpOneIterInitEv
    //
    // and then pass this to abi::__cxa_demangle to get the demanged name:
    // HPHP::JIT::interpOneIterInit()
    //
    // Sometimes, though, backtrace_symbols can't find the function name
    // and ends up giving us a blank managled name, like this:
    // ../path/hhvm/hhvm() [0x17e4d01]
    // or this: [0x7fffca800130]
    //
    char* start = strchr(*symbols, '(');
    if (start) {
      start++;
      char* end = strchr(start, '+');
      if (end != nullptr) {
        size_t len = end-start;
        functionName = new char[len+1];
        strncpy(functionName, start, len);
        functionName[len] = '\0';
        int status;
        char* demangledName = abi::__cxa_demangle(functionName, 0, 0, &status);
        if (status == 0) {
          delete [] functionName;

          // Callers expect the buffer to have come from new[] but demangledName
          // came from malloc. Copy to a buffer from new[].
          auto const len = strlen(demangledName) + 1;
          char* buf = new char[len];
          std::copy(demangledName, demangledName + len, buf);
          free(demangledName);
          functionName = buf;
        }
      }
    }
  }
  free(symbols);
  if (functionName == nullptr) {
#define MAX_ADDR_HEX_LEN 40
    functionName = new char[MAX_ADDR_HEX_LEN + 3];
    std::sprintf(functionName, "%40p", codeAddr);
  }

  {
    G g(nameCacheLock);
    nameCache[codeAddr] = std::string(functionName);
  }
  return functionName;
}

//////////////////////////////////////////////////////////////////////

}

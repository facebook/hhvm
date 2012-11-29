/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#ifndef incl_HPHP_RINGBUFFER_H_
#define incl_HPHP_RINGBUFFER_H_

#include <cstdlib>
#include <cstdarg>
#include <cinttypes>

namespace HPHP {
namespace Trace {

#define RBTYPES \
  RBTYPE(Uninit) \
  RBTYPE(Msg) \
  RBTYPE(SideExit) \
  RBTYPE(TraceletBody) \
  RBTYPE(TraceletGuards) \
  RBTYPE(FuncEntry) \
  RBTYPE(FuncExit) \
  RBTYPE(FuncPrologueTry)

enum RingBufferType {
#define RBTYPE(x) RBType ## x,
  RBTYPES
#undef RBTYPE
};

void vtraceRingbuffer(const char* fmt, va_list ap);
void ringbufferMsg(const char* msg, size_t msgLen, RingBufferType t = RBTypeMsg);
void ringbufferEntry(RingBufferType t, uint64_t funcId, int offset);

}
}

#endif

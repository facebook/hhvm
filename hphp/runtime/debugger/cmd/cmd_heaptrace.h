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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_HEAPTRACE_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_HEAPTRACE_H_

#include <map>
#include <vector>

#include "hphp/runtime/debugger/cmd/cmd_extended.h"

#include "hphp/runtime/base/tracer.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

struct GraphFormat {
  const std::string prologue;
  const std::function<std::string(TypedValue *, const char *)> stringifyNode;
  const std::function<std::string(TypedValue *, TypedValue *)> stringifyEdge;
  const std::string epilogue;
};

DECLARE_BOOST_TYPES(CmdHeaptrace);
class CmdHeaptrace : public CmdExtended {
public:
  virtual void list(DebuggerClient &client);
  virtual void help(DebuggerClient &client);
  virtual bool onServer(DebuggerProxy &proxy);
  virtual void onClient(DebuggerClient &client);

protected:
  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

private:
  void printHeap(DebuggerClient &client);
  void printGraphToFile(DebuggerClient &client,
                        String filename,
                        const GraphFormat &gf);

  struct Accum {
    std::map<int64_t, int8_t> typesMap;
    std::map<int64_t, int64_t> sizeMap;
    std::map<int64_t, std::vector<int64_t>> adjacencyList;
  } m_accum;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_CMD_HEAPTRACE_H__

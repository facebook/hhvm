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

#include "hphp/runtime/debugger/cmd/cmd_where.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_asio.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"
#include "hphp/runtime/ext/generator/ext_generator.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debugger);

void CmdWhere::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  {
    String sdata;
    DebuggerWireHelpers::WireSerialize(m_stacktrace, sdata);
    thrift.write(sdata);
  }
  thrift.write(m_stackArgs);
}

void CmdWhere::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  {
    String sdata;
    thrift.read(sdata);
    if (DebuggerWireHelpers::WireUnserialize(sdata, m_stacktrace) !=
        DebuggerWireHelpers::NoError) {
      m_stacktrace.reset();
      m_wireError = sdata;
    }
  }
  thrift.read(m_stackArgs);
}

void CmdWhere::help(DebuggerClient &client) {
  client.helpTitle("Where Command");
  client.helpCmds(
    "[w]here", "displays current stacktrace",
    "[w]here [a]sync", "displays the current async stacktrace",
    "wa", "shortcut for [w]here [a]sync",
    "[w]here {[a]sync]} {num}", "displays number of innermost frames",
    "[w]here {[a]sync]} -{num}", "displays number of outermost frames",
    nullptr
  );
  client.helpBody(
    "Use '[u]p {num}' or '[d]own {num}' to walk up or down the stacktrace. "
    "Use '[f]rame {index}' to jump to one particular frame. At any frame, "
    "Use '[v]ariable' command to display all local variables.\n"
    "\n"
    "Use '[w]here [a]sync' from within an async method, like a generator, "
    "to get the current stack of async methods."
  );
}

Array CmdWhere::fetchStackTrace(DebuggerClient &client) {
  Array st = client.getStackTrace();
  // Only grab a new stack trace if we don't have one cached, or if
  // the one cached does not match the type of stack trace being
  // requested.
  bool isAsync = m_type == KindOfWhereAsync;
  if (st.isNull() || (isAsync != client.isStackTraceAsync())) {
    m_stackArgs = client.getDebuggerClientStackArgs();
    auto cmd = client.xend<CmdWhere>(this);
    st = cmd->m_stacktrace;
    client.setStackTrace(st, isAsync);
  }
  return st;
}

void CmdWhere::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;
  if (client.argCount() > 2) {
    help(client);
    return;
  }
  int argBase = 1;
  if ((client.argCount() > 0) && client.arg(argBase, "async")) {
    // We use a different command type for an async stack trace, so we
    // can both send and receive different data and still keep the
    // existing Where command unchanged. This ensures that old clients
    // can still get a stack trace from a newer server, and vice
    // versa.
    m_type = KindOfWhereAsync;
    argBase++;
    client.info("Fetching async stacktrace...");
  }

  Array st = fetchStackTrace(client);
  if (st.empty()) {
    if (m_type != KindOfWhereAsync) {
      client.info("(no stacktrace to display or in global scope)");
      client.info("If you hit the serialization limit, try "
                  "\"set sa off\" to get the stack without args");
    } else {
      client.info("(no async stacktrace to display)");
    }
    return;
  }

  // so list command can default to current frame
  client.moveToFrame(client.getFrame(), false);

  if (client.argCount() < argBase) {
    int i = 0;
    for (ArrayIter iter(st); iter; ++iter) {
      client.printFrame(i, iter.second().toArray());
      ++i;
      if (i % DebuggerClient::ScrollBlockSize == 0 &&
          client.ask("There are %zd more frames. Continue? [Y/n]",
                      st.size() - i) == 'n') {
        break;
      }
    }
  } else {
    std::string snum = client.argValue(argBase);
    int num = atoi(snum.c_str());
    if (snum[0] == '-') {
      snum = snum.substr(1);
    }
    if (!DebuggerClient::IsValidNumber(snum)) {
      client.error("The argument, if specified, has to be numeric.");
      return;
    }
    if (num > 0) {
      for (int i = 0; i < num && i < st.size(); i++) {
        client.printFrame(i, st[i].toArray());
      }
    } else if (num < 0) {
      for (int i = st.size() + num; i < st.size(); i++) {
        client.printFrame(i, st[i].toArray());
      }
    } else {
      client.error("0 was specified for the number of frames");
      client.tutorial(
        "The optional argument is the number of frames to print out. "
        "Use a positive number to print out innermost frames. Use a negative "
        "number to print out outermost frames."
      );
    }
  }
}

c_WaitableWaitHandle *objToWaitableWaitHandle(Object o) {
  assert(o->instanceof(c_WaitableWaitHandle::classof()));
  return static_cast<c_WaitableWaitHandle*>(o.get());
}

const StaticString
  s_function("function"),
  s_id("id"),
  s_file("file"),
  s_line("line"),
  s_ancestors("ancestors");

// Add location information for the given continuation to the given frame.
void addAsyncFunctionLocation(Array& frameData,
                              c_AsyncFunctionWaitHandle& wh) {
  // A running async function is active on the normal stack, and we
  // cannot compute the location just by inspecting the async function
  // wait handle alone.
  if (wh.isRunning()) return;
  frameData.set(s_file, wh.getFileName(), true);
  frameData.set(s_line, wh.getLineNumber(), true);
}

// Form a trace of the async stack starting with the currently running
// generator, if any. For now we just toss in the function name and
// id, as well as the pseudo-frames for context breaks at explicit
// joins. Later we'll add more, like file and line, hopefully function
// args, wait handle status, etc.
static Array createAsyncStacktrace() {
  Array trace;
  auto currentWaitHandle = HHVM_FN(asio_get_running)();
  if (currentWaitHandle.isNull()) return trace;
  Array depStack =
    objToWaitableWaitHandle(currentWaitHandle)->t_getdependencystack();
  for (ArrayIter iter(depStack); iter; ++iter) {
    if (iter.secondRef().isNull()) {
      trace.append(Array(staticEmptyArray()));
    } else {
      auto wh = objToWaitableWaitHandle(iter.secondRef().toObject());
      auto parents = wh->t_getparents();
      Array ancestors;
      for (ArrayIter piter(parents); piter; ++piter) {
        // Note: the parent list contains no nulls.
        auto parent = objToWaitableWaitHandle(piter.secondRef().toObject());
        ancestors.append(parent->t_getname());
      }
      Array frameData;
      frameData.set(s_function, wh->t_getname(), true);
      frameData.set(s_id, wh->t_getid(), true);
      frameData.set(s_ancestors, ancestors, true);
      // Async function wait handles may have a source location to add.
      if (wh->getKind() == c_WaitHandle::Kind::AsyncFunction) {
        auto afwh = static_cast<c_AsyncFunctionWaitHandle*>(wh);
        addAsyncFunctionLocation(frameData, *afwh);
      }
      trace.append(frameData);
    }
  }
  return trace;
}

bool CmdWhere::onServer(DebuggerProxy &proxy) {
  if (m_type == KindOfWhereAsync) {
    m_stacktrace = createAsyncStacktrace();
  } else {
    m_stacktrace = createBacktrace(BacktraceArgs()
                                   .withSelf()
                                   .ignoreArgs(!m_stackArgs));
  }
  return proxy.sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////
}}

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/debugger/cmd/cmd_info.h"
#include "hphp/runtime/ext/reflection/ext_reflection.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/command.h"

namespace HPHP {
namespace VSDEBUG {

InfoCommand::InfoCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message) {
}

InfoCommand::~InfoCommand() {
}

static void tryAddClassInfo(
  const std::string& clsName,
  const std::string& funcName,
  StringBuffer& sb
) {
  try {
    Array ret = DebuggerReflection::get_class_info(clsName);
    if (!ret.empty()) {
      Eval::CmdInfo::PrintInfo(
        nullptr,
        sb,
        ret,
        funcName.empty() ? clsName : funcName
      );
    }
  } catch (...) {}
}

static void tryAddFunctionInfo(
  const std::string& funcName,
  StringBuffer& sb
) {
  try {
    Array ret = DebuggerReflection::get_function_info(funcName);
    if (!ret.empty()) {
      Eval::CmdInfo::PrintInfo(nullptr, sb, ret, funcName);
    }
  } catch (...) {}
}

bool InfoCommand::executeImpl(
  DebuggerSession* /*session*/,
  folly::dynamic* responseMsg
) {
  VMRegAnchor regAnchor;

  // The request thread should not re-enter the debugger while
  // processing this command.
  DebuggerNoBreakContext noBreak(m_debugger);

  const folly::dynamic& message = getMessage();
  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  std::string requestedObject = tryGetString(args, "object", "");
  StringBuffer sb;

  if (requestedObject.empty()) {
    // If no object is requested, we return info about this thread's current
    // stop location.
    auto const frame = g_context->getFrameAtDepthForDebuggerUnsafe(0);
    if (frame == nullptr) {
      throw DebuggerCommandException(
        "No object specified and the target thread is not stopped in a frame"
      );
    }

    const Func* func = frame->func();
    if (func == nullptr) {
      throw DebuggerCommandException(
        "No object specified and the target thread is not in a function"
      );
    }

    auto const funcName = func->name()->data();
    if (func->cls() != nullptr) {
      // If the current location is a class method, print the method info.
      tryAddClassInfo(func->cls()->name()->data(), funcName, sb);
    } else {
      // Otherwise return info for the current function.
      tryAddFunctionInfo(funcName, sb);
    }
  } else {
    std::string subSymbol = "";
    size_t pos = std::string::npos;
    if (requestedObject.size() >= 2) {
      pos = requestedObject.find("::");
    }
    if (pos != std::string::npos) {
      subSymbol = requestedObject.substr(pos + 2);
      requestedObject = requestedObject.substr(0, pos);
    }
    tryAddClassInfo(requestedObject, subSymbol, sb);
    tryAddFunctionInfo(requestedObject, sb);
  }

  if (sb.empty()) {
    throw DebuggerCommandException(
      "Couldn't find any info for the requested object."
    );
  }

  (*responseMsg)["body"] = folly::dynamic::object;
  (*responseMsg)["body"]["info"] = std::string(sb.data());

  // This command does not resume the target.
  return false;
}

}
}

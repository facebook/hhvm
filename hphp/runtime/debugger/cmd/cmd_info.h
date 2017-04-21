/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_INFO_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_INFO_H_

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/runtime/debugger/debugger_command.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

struct CmdInfo : DebuggerCommand {
  static void UpdateLiveLists(DebuggerClient &client);
  static String GetProtoType(DebuggerClient &client, const std::string &cls,
                             const std::string &func);

  CmdInfo() : DebuggerCommand(KindOfInfo) {}

  void list(DebuggerClient&) override;
  void help(DebuggerClient&) override;

  bool onServer(DebuggerProxy&) override;
  void onClient(DebuggerClient&) override;

  bool parseZeroArg(DebuggerClient &client);
  void parseOneArg(DebuggerClient &client, std::string &subsymbol);

  Array getInfo() const {
    return m_info;
  }

  static String FindSubSymbol(const Array& symbols, const std::string &symbol);

protected:
  void sendImpl(DebuggerThriftBuffer&) override;
  void recvImpl(DebuggerThriftBuffer&) override;

private:
  enum SymbolType {
    KindOfUnknown,
    KindOfClass,
    KindOfFunction,

    KindOfLiveLists, // for auto-complete
  };

  int8_t m_type;
  req::root<String> m_symbol;
  req::root<Array>  m_info;
  std::shared_ptr<DebuggerClient::LiveLists> m_acLiveLists;

  static String GetParams(const Array& params, bool varg, bool detailed=false);
  static String GetParam(const Array& params, int index);
  static String GetModifier(const Array& info, const String&);
  static String GetTypeProfilingInfo(const Array& profilingArray,
                                     const Array& params);

  static bool TryConstant(StringBuffer &sb, const Array& info,
                          const std::string &subsymbol);
  static bool TryProperty(StringBuffer &sb, const Array& info,
                          const std::string &subsymbol);
  static bool TryMethod(DebuggerClient &client, StringBuffer &sb,
                        const Array& info, std::string subsymbol);

  static void PrintDocComments(StringBuffer &sb, const Array& info);
  static void PrintInfo(DebuggerClient &client, StringBuffer &sb,
                        const Array& info,
                        const std::string &subsymbol);
  static void PrintHeader(DebuggerClient &client, StringBuffer &sb,
                          const Array& info);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_INFO_H_

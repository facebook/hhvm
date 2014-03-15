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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_INFO_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_INFO_H_

#include "hphp/runtime/debugger/debugger_command.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class CmdInfo : public DebuggerCommand {
public:
  static void UpdateLiveLists(DebuggerClient &client);
  static String GetProtoType(DebuggerClient &client, const std::string &cls,
                             const std::string &func);

public:
  CmdInfo() : DebuggerCommand(KindOfInfo) {}

  virtual void list(DebuggerClient &client);
  virtual void help(DebuggerClient &client);

  virtual bool onServer(DebuggerProxy &proxy);
  virtual void onClient(DebuggerClient &client);

  bool parseZeroArg(DebuggerClient &client);
  void parseOneArg(DebuggerClient &client, std::string &subsymbol);
  Array getInfo() { return m_info; }
  static String FindSubSymbol(const Array& symbols, const std::string &symbol);

protected:
  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

private:
  enum SymbolType {
    KindOfUnknown,
    KindOfClass,
    KindOfFunction,

    KindOfLiveLists, // for auto-complete
  };

  int8_t   m_type;
  String m_symbol;
  Array  m_info;
  DebuggerClient::LiveListsPtr m_acLiveLists;

  static String GetParams(const Array& params, bool varg, bool detailed = false);
  static String GetParam(const Array& params, int index);
  static String GetModifier(const Array& info, const String&);
  static String GetTypeProfilingInfo(const Array& profilingArray, const Array& params);

  static bool TryConstant(StringBuffer &sb, const Array& info,
                          const std::string &subsymbol);
  static bool TryProperty(StringBuffer &sb, const Array& info,
                          const std::string &subsymbol);
  static bool TryMethod(DebuggerClient &client, StringBuffer &sb,
                        const Array& info, std::string subsymbol);

  static void PrintDocComments(StringBuffer &sb, const Array& info);
  static void PrintInfo(DebuggerClient &client, StringBuffer &sb, const Array& info,
                        const std::string &subsymbol);
  static void PrintHeader(DebuggerClient &client, StringBuffer &sb,
                          const Array& info);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_INFO_H_

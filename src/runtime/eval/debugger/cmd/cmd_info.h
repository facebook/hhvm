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

#ifndef __HPHP_EVAL_DEBUGGER_CMD_INFO_H__
#define __HPHP_EVAL_DEBUGGER_CMD_INFO_H__

#include <runtime/eval/debugger/debugger_command.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdInfo);
class CmdInfo : public DebuggerCommand {
public:
  static void UpdateLiveLists(DebuggerClient *client);
  static String GetProtoType(DebuggerClient *client, const std::string &cls,
                             const std::string &func);

public:
  CmdInfo() : DebuggerCommand(KindOfInfo) {}

  virtual void list(DebuggerClient *client);
  virtual bool help(DebuggerClient *client);

  virtual bool onClient(DebuggerClient *client);
  virtual bool onServer(DebuggerProxy *proxy);

  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

private:
  enum SymbolType {
    KindOfUnknown,
    KindOfClass,
    KindOfFunction,

    KindOfLiveLists, // for auto-complete
  };

  int8   m_type;
  String m_symbol;
  Array  m_info;
  DebuggerClient::LiveListsPtr m_acLiveLists;

  static String GetParams(CArrRef params, bool varg, bool detailed = false);
  static String GetModifier(CArrRef info, const char *name);

  static String FindSubSymbol(CArrRef symbols, const std::string &symbol);

  static bool TryConstant(StringBuffer &sb, CArrRef info,
                          const std::string &subsymbol);
  static bool TryProperty(StringBuffer &sb, CArrRef info,
                          const std::string &subsymbol);
  static bool TryMethod(DebuggerClient *client, StringBuffer &sb,
                        CArrRef info, std::string subsymbol);

  static void PrintDocComments(StringBuffer &sb, CArrRef info);
  static void PrintHeader(DebuggerClient *client, StringBuffer &sb,
                          CArrRef info);
  static void PrintInfo(DebuggerClient *client, StringBuffer &sb, CArrRef info,
                        const std::string &subsymbol);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_CMD_INFO_H__

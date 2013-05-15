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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_LIST_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_LIST_H_

#include "hphp/runtime/eval/debugger/debugger_command.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdList);
class CmdList : public DebuggerCommand {
public:
  CmdList() : DebuggerCommand(KindOfList) {}

  // Sends a "list file" command to the proxy attached to the given client.
  // Returns false if the file does not exist or could not be read or an
  // HPHP::String instance containing the contents of the file.
  static Variant GetSourceFile(DebuggerClient *client,
                               const std::string &file);

  // Informs the client of all strings that may follow a list command.
  // Used for auto completion. The client uses the prefix of the argument
  // following the command to narrow down the list displayed to the user.
  virtual void list(DebuggerClient *client);

  // The text to display when the debugger client processes "help break".
  virtual bool help(DebuggerClient *client);

  // Puts the specified range of the contents of the source file referenced
  // by this command in m_code and sends a copy of the updated command back
  // to the client.
  virtual bool onServer(DebuggerProxy *proxy);

protected:
  // Verifies the arguments of this command, sends the command to the
  // server to get back the listing, updates the client with the current
  // position in the source file and displays a list of source lines to
  // the console.
  virtual bool onClientImpl(DebuggerClient *client);

  // Serializes this command into the given Thrift buffer.
  virtual void sendImpl(DebuggerThriftBuffer &thrift);

  // Deserializes a CmdList from the given Thrift buffer.
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

private:
  // A path to a source file. If relative this is relative to url
  // loaded into the server (if any).
  std::string m_file;

  // The first line of the range of source lines to be listed.
  int32_t m_line1;

  //The last line of the range of source lines to be listed.
  int32_t m_line2;

  // If null, this is uninitialized. If false, there is no such range/file.
  // Otherwise, this contains an HPHP::String instance representing the
  // range of source text to be listed by this command.
  Variant m_code;

  bool listCurrent(DebuggerClient *client, int &line,
                   int &charFocus0, int &lineFocus1,
                   int &charFocus1);

  bool listFileRange(DebuggerClient *client, int line,
                     int charFocus0, int lineFocus1,
                     int charFocus1);

  bool listFunctionOrClass(DebuggerClient *client);

};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_LIST_H_

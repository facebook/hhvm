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

#include <runtime/eval/debugger/cmd/cmd_print.h>
#include <runtime/base/time/datetime.h>
#include <runtime/base/string_util.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

const char *CmdPrint::Formats[] = {
  "x", "hex", "oct", "dec", "unsigned", "time", NULL
};

std::string CmdPrint::FormatResult(const char *format, CVarRef ret) {
  if (format == NULL) {
    String sret = DebuggerClient::FormatVariable(ret, -1);
    return string(sret.data(), sret.size());
  }

  if (strcmp(format, "dec") == 0 ||
      strcmp(format, "unsigned") == 0 ||
      ret.isInteger()) {
    int64 nret = ret.toInt64();
    char buf[64];
    if (strcmp(format, "hex") == 0 || strcmp(format, "x") == 0) {
      snprintf(buf, sizeof(buf), "%llx", nret);
      return buf;
    }
    if (strcmp(format, "oct") == 0) {
      snprintf(buf, sizeof(buf), "%llo", nret);
      return buf;
    }
    if (strcmp(format, "dec") == 0) {
      snprintf(buf, sizeof(buf), "%lld", nret);
      return buf;
    }
    if (strcmp(format, "unsigned") == 0) {
      snprintf(buf, sizeof(buf), "%llu", (unsigned long long)nret);
      return buf;
    }
    if (strcmp(format, "time") == 0) {
      StringBuffer sb;
      DateTime dt(nret);
      sb.append("RFC822:            ");
      sb.append(dt.toString(DateTime::RFC822));
      sb.append("\nRFC850:            ");
      sb.append(dt.toString(DateTime::RFC850));
      sb.append("\nRFC1036:           ");
      sb.append(dt.toString(DateTime::RFC1036));
      sb.append("\nRFC1123/RSS:       ");
      sb.append(dt.toString(DateTime::RFC1123));
      sb.append("\nRFC2822:           ");
      sb.append(dt.toString(DateTime::RFC2822));
      sb.append("\nRFC3339/ATOM/W3C:  ");
      sb.append(dt.toString(DateTime::RFC3339));
      sb.append("\nISO8601:           ");
      sb.append(dt.toString(DateTime::ISO8601));
      sb.append("\nCookie:            ");
      sb.append(dt.toString(DateTime::Cookie));
      sb.append("\nHttpHeader:        ");
      sb.append(dt.toString(DateTime::HttpHeader));
      return sb.data();
    }

    ASSERT(false);
  }

  String sret = DebuggerClient::FormatVariable(ret, -1);
  if (strcmp(format, "hex") == 0 || strcmp(format, "x") == 0 ||
      strcmp(format, "oct") == 0) {
    StringBuffer sb;
    for (int i = 0; i < sret.size(); i++) {
      char ch = sret[i];
      if (isprint(ch)) {
        sb.append(ch);
      } else {
        char buf[6];
        if (strcmp(format, "oct") == 0) {
          snprintf(buf, sizeof(buf), "\\%03o", ch);
        } else {
          snprintf(buf, sizeof(buf), "\\x%02x", ch);
        }
        sb.append(buf);
      }
    }
    return sb.data();
  }
  if (strcmp(format, "time") == 0) {
    DateTime dt;
    int64 ts = -1;
    if (dt.fromString(ret.toString(), SmartObject<TimeZone>())) {
      bool err;
      ts = dt.toTimeStamp(err);
    }
    return String(ts).data();
  }

  ASSERT(false);
  return "";
}

///////////////////////////////////////////////////////////////////////////////

void CmdPrint::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_ret);
  thrift.write(m_output);
  thrift.write(m_frame);
}

void CmdPrint::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_ret);
  thrift.read(m_output);
  thrift.read(m_frame);
}

void CmdPrint::list(DebuggerClient *client) {
  if (client->arg(1, "clear")) {
    client->addCompletion("all");
    return;
  }
  client->addCompletion(DebuggerClient::AutoCompleteCode);

  if (client->argCount() == 0) {
    client->addCompletion(Formats);
    client->addCompletion("always");
    client->addCompletion("list");
    client->addCompletion("clear");
  } else if (client->argCount() == 1 && client->arg(1, "always")) {
    client->addCompletion(Formats);
  }
}

bool CmdPrint::help(DebuggerClient *client) {
  client->helpTitle("Print Command");
  client->helpCmds(
    "[p]rint {php}",              "prints result of PHP code",
    "[p]rint x {php}",            "prints hex encoded string or number",
    "[p]rint [h]ex {php}",        "prints hex encoded string or number",
    "[p]rint [o]ct {php}",        "prints octal encoded string or number",
    "[p]rint [d]ec {php}",        "prints as signed integer",
    "[p]rint [u]nsigned {php}",   "prints as unsigned integer",
    "[p]rint [t]ime {php}",       "converts between time and timestamp",
    "",                           "",
    "[p]rint [a]lways {above}",   "adds a watch expression at break",
    "[p]rint [l]ist",             "lists watch expressions",
    "[p]rint [c]lear {index}",    "clears a watch expression",
    "[p]rint [c]lear [a]ll",      "clears all watch expressions",
    NULL
  );
  client->helpBody(
    "Prints result of an expression in certain format. If '[a]lways' is "
    "specified, the expression will be added to a watch list. At every break, "
    "either at a breakpoint or caused by step commands, these expressions "
    "will be evaluated and printed out."
  );
  return true;
}

bool CmdPrint::processList(DebuggerClient *client) {
  DebuggerClient::WatchPtrVec &watches = client->getWatches();
  for (int i = 0; i < (int)watches.size(); i++) {
    client->print("  %d %s  %s", i + 1,
                  StringUtil::Pad(watches[i]->first, 8, " ",
                                  StringUtil::PadLeft).data(),
                  watches[i]->second.c_str());
  }
  if (watches.empty()) {
    client->tutorial(
      "Use '[p]rint [a]lways ...' to set new watch expressions. "
      "Use '[p]rint ?|[h]elp' to read how to set them. "
    );
  } else {
    client->tutorial(
      "Use '[p]rint [c]lear {index}|[a]ll' to remove watch expression(s). "
    );
  }
  return true;
}

bool CmdPrint::processClear(DebuggerClient *client) {
  DebuggerClient::WatchPtrVec &watches = client->getWatches();
  if (watches.empty()) {
    client->error("There is no watch expression to clear.");
    client->tutorial(
      "Use '[p]rint [a]lways ...' to set new watch expressions. "
      "Use '[p]rint ?|[h]elp' to read how to set them. "
    );
    return true;
  }

  if (client->arg(2, "all")) {
    watches.clear();
    client->info("All watch expressions are cleared.");
    return true;
  }

  string snum = client->argValue(2);
  if (!DebuggerClient::IsValidNumber(snum)) {
    client->error("'[p]rint [c]lear' needs an {index} argument.");
    client->tutorial(
      "You will have to run '[p]rint [l]ist' first to see a list of valid "
      "numbers or indices to specify."
    );
    return true;
  }

  int num = atoi(snum.c_str()) - 1;
  if (num < 0 || num >= (int)watches.size()) {
    client->error("\"%s\" is not a valid index. Choose one from this list:",
                  snum.c_str());
    processList(client);
    return true;
  }

  watches.erase(watches.begin() + num);
  return true;
}

void CmdPrint::processWatch(DebuggerClient *client, const char *format,
                            const std::string &php) {
  m_body = php;
  m_frame = client->getFrame();
  CmdPrintPtr res = client->xend<CmdPrint>(this);
  if (!res->m_output.empty()) {
    client->output(res->m_output);
  }
  client->output(FormatResult(format, res->m_ret));
}

bool CmdPrint::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;
  if (client->argCount() == 0) {
    return help(client);
  }

  bool watch = false;
  int index = 1;
  if (client->arg(1, "always")) {
    if (client->argCount() == 1) {
      client->error("'[p]rint [a]lways' needs an expression to watch.");
      return true;
    }
    watch = true;
    index++;
  } else if (client->arg(1, "list")) {
    return processList(client);
  } else if (client->arg(1, "clear")) {
    return processClear(client);
  }

  const char *format = NULL;
  for (const char **fmt = Formats; *fmt; fmt++) {
    if (client->arg(index, *fmt)) {
      format = *fmt;
      index++;
      break;
    }
  }
  m_body = client->argRest(index);
  if (watch) {
    client->addWatch(format, m_body);
  }
  processWatch(client, format, m_body);
  return true;
}

bool CmdPrint::onServer(DebuggerProxy *proxy) {
  m_ret = DebuggerProxy::ExecutePHP(DebuggerProxy::MakePHPReturn(m_body),
                                    m_output, !proxy->isLocal(), m_frame);
  return proxy->send(this);
}

///////////////////////////////////////////////////////////////////////////////
}}

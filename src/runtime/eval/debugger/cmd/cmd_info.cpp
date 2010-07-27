/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/eval/debugger/cmd/cmd_info.h>
#include <runtime/ext/ext_reflection.h>
#include <runtime/base/preg.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdInfo::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_type);
  thrift.write(m_symbol);
  thrift.write(m_info);
}

void CmdInfo::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_type);
  thrift.read(m_symbol);
  thrift.read(m_info);
}

bool CmdInfo::help(DebuggerClient *client) {
  client->helpTitle("Info Command");
  client->help("info                   displays current function's info");
  client->help("info {cls}             displays declaration of this class");
  client->help("info {function}        displays declaration of this function");
  client->help("info {cls::method}     displays declaration of this method");
  client->help("info {cls::name}       displays declaration of this constant");
  client->help("info {cls::$property}  displays declaration of this property");
  client->helpBody(
    "Use this command to display declaration of a symbol."
  );
  return true;
}

bool CmdInfo::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

  string subsymbol;

  if (client->argCount() == 0) {
    BreakPointInfoPtr bpi = client->getCurrentLocation();
    if (bpi) {
      m_symbol = bpi->m_class;
      m_type = KindOfClass;
      if (m_symbol.empty()) {
        m_symbol = bpi->m_function;
        m_type = KindOfFunction;
      }
    }
    if (m_symbol.empty()) {
      client->error("There is no current function or method to look up.");
      client->tutorial(
        "You can only use '[i]nfo' without a symbol name when you are running "
        "your program and it breaks at a function or a class method. It will "
        "then look up information about that function or method."
      );
      return true;
    }
  } else if (client->argCount() == 1) {
    string symbol = client->argValue(1);
    size_t pos = symbol.find("::");
    if (pos != string::npos) {
      m_symbol = String(symbol.substr(0, pos));
      m_type = KindOfClass;
      subsymbol = symbol.substr(pos + 2);
    } else if (symbol.size() > 2 && symbol.substr(symbol.size() - 2) == "()") {
      m_symbol = symbol.substr(0, symbol.size() - 2);
      m_type = KindOfFunction;
    } else {
      m_symbol = String(symbol);
      m_type = KindOfUnknown;
    }
  } else {
    return help(client);
  }

  CmdInfoPtr cmd = client->xend<CmdInfo>(this);
  Array info = cmd->m_info;
  if (info.empty()) {
    client->info("(specified symbol cannot be found)");
  } else {
    for (ArrayIter iter(info); iter; ++iter) {
      PrintInfo(client, iter.second(), subsymbol);
    }
  }

  return true;
}

bool CmdInfo::onServer(DebuggerProxy *proxy) {
  if (m_type == KindOfUnknown || m_type == KindOfClass) {
    Array ret = f_hphp_get_class_info(m_symbol);
    if (!ret.empty()) {
      m_info.append(ret);
    }
  }
  if (m_type == KindOfUnknown || m_type == KindOfFunction) {
    Array ret = f_hphp_get_function_info(m_symbol);
    if (!ret.empty()) {
      m_info.append(ret);
    }
  }
  return proxy->send(this);
}

///////////////////////////////////////////////////////////////////////////////

void CmdInfo::PrintDocComments(DebuggerClient *client, CArrRef info) {
  if (info["doc"].isString()) {
    String doc = info["doc"].toString();
    int space1 = 0; // best guess
    int space2 = 3; // best guess
    Variant matches1, matches2;
    Variant ret1 = preg_match("#^( *)/\\*#s", doc, matches1);
    Variant ret2 = preg_match("#\n( *)\\*#s", doc, matches2);
    if (!same(ret1, false) && !same(ret2, false)) {
      // we have perfect doc comment blocks, so we can re-adjust spaces
      space1 = matches1[1].toString().size();
      space2 = matches2[1].toString().size();
    }
    String spaces = StringUtil::Repeat(" ", space2 - space1 - 1);
    client->comment("%s%s", spaces.data(), doc.data());
  }
}

void CmdInfo::PrintHeader(DebuggerClient *client, CArrRef info,
                          const char *type) {
  if (info["internal"].toBoolean()) {
    if (info["hphp"].toBoolean()) {
      client->comment("// HipHop builtin %s", type);
    } else {
      client->comment("// PHP builtin %s: "
                      "http://php.net/manual/en/%s.%s.php",
                      type, type, StringUtil::ToLower(info["name"]).data());
    }
  } else {
    String file = info["file"].toString();
    int line = info["line1"].toInt32();
    if (file.empty() && line == 0) {
      client->comment("// user %s (source unknown)", type);
    } else if (line == 0) {
      client->comment("// defined in %s", file.data());
    } else {
      client->comment("// defined on line %d of %s", line, file.data());
    }
  }

  PrintDocComments(client, info);
}

String CmdInfo::GetParams(CArrRef params, bool detailed /* = false */) {
  StringBuffer args;
  for (ArrayIter iter(params); iter; ++iter) {
    if (!args.empty()) {
      args.append(", ");
    }
    Array arg = iter.second().toArray();
    if (!arg["type"].toString().empty()) {
      args.append(arg["type"].toString());
      args.append(' ');
    }
    if (arg["ref"].toBoolean()) {
      args.append('&');
    }
    args.append('$');
    args.append(arg["name"].toString());
    if (arg.exists("default")) {
      args.append(" = ");
      if (detailed) {
        args.append(DebuggerClient::PrintVariable(arg["default"], -1));
      } else {
        args.append(DebuggerClient::PrintVariable(arg["default"]));
      }
    }
  }
  return args.detach();
}

String CmdInfo::GetModifier(CArrRef info, const char *name) {
  if (info[name].toBoolean()) {
    return String(name) + " ";
  }
  return "";
}

String CmdInfo::FindSubSymbol(CArrRef symbols, const std::string &symbol) {
  for (ArrayIter iter(symbols); iter; ++iter) {
    String key = iter.first().toString();
    if (strcasecmp(key.data(), symbol.c_str()) == 0) {
      return key;
    }
  }
  return String();
}

bool CmdInfo::TryConstant(DebuggerClient *client, CArrRef info,
                          const std::string &subsymbol) {
  String key = FindSubSymbol(info["constants"], subsymbol);
  if (!key.isNull()) {
    client->help("  const %s = %s;", key.data(),
                 DebuggerClient::PrintVariable
                 (info["constants"][key], -1).data());
    return true;
  }
  return false;
}

bool CmdInfo::TryProperty(DebuggerClient *client, CArrRef info,
                          const std::string &subsymbol) {
  String key = FindSubSymbol(info["properties"],
                             subsymbol[0] == '$' ?
                             subsymbol.substr(1) : subsymbol);
  if (!key.isNull()) {
    Array prop = info["properties"][key];
    PrintDocComments(client, prop);
    client->help("  %s %s$%s;",
                 prop["access"].toString().data(),
                 GetModifier(prop, "static").data(),
                 prop["name"].toString().data());
    return true;
  }
  return false;
}

bool CmdInfo::TryMethod(DebuggerClient *client, CArrRef info,
                        std::string subsymbol) {
  if (subsymbol.size() > 2 && subsymbol.substr(subsymbol.size() - 2) == "()") {
    subsymbol = subsymbol.substr(0, subsymbol.size() - 2);
  }

  String key = FindSubSymbol(info["methods"], subsymbol);
  if (!key.isNull()) {
    Array func = info["methods"][key].toArray();
    PrintDocComments(client, func);
    client->help("  %s %s%s%sfunction %s%s(%s);",
                 func["access"].toString().data(),
                 GetModifier(func, "static").data(),
                 GetModifier(func, "final").data(),
                 GetModifier(func, "abstract").data(),
                 func["ref"].toBoolean() ? "&" : "",
                 func["name"].toString().data(),
                 GetParams(func["params"]).data(), true);
    return true;
  }
  return false;
}

void CmdInfo::PrintInfo(DebuggerClient *client, CArrRef info,
                        const std::string &subsymbol) {
  if (info.exists("params")) {
    PrintHeader(client, info, "function");
    client->help("function %s%s(%s);",
                 info["ref"].toBoolean() ? "&" : "",
                 info["name"].toString().data(),
                 GetParams(info["params"]).data());
    return;
  }

  bool found = false;
  if (!subsymbol.empty()) {
    if (TryConstant(client, info, subsymbol)) found = true;
    if (TryProperty(client, info, subsymbol)) found = true;
    if (TryMethod(client, info, subsymbol)) found = true;
    if (found) return;
    client->info("Specified symbol cannot be found. Here the whole class:\n");
  }

  PrintHeader(client, info, "class");

  StringBuffer parents;
  String parent = info["parent"].toString();
  if (!parent.empty()) {
    parents.append("extends ");
    parents.append(parent);
    parents.append(' ');
  }
  if (!info["interfaces"].toArray().empty()) {
    parents.append("implements ");
    bool first = true;
    for (ArrayIter iter(info["interfaces"]); iter; ++iter) {
      if (first) {
        first = false;
      } else {
        parents.append(", ");
      }
      parents.append(iter.first().toString());
    }
    parents.append(' ');
  }
  parent = parents.detach();

  client->help("%s%s%s %s %s{",
               GetModifier(info, "final").data(),
               GetModifier(info, "abstract").data(),
               info["interface"].toBoolean() ? "interface" : "class",
               info["name"].toString().data(),
               parent.data());

  if (!info["constants"].toArray().empty()) {
    client->comment("  // constants");
    for (ArrayIter iter(info["constants"]); iter; ++iter) {
      client->help("  const %s = %s;",
                   iter.first().toString().data(),
                   DebuggerClient::PrintVariable(iter.second()).data());
    }
  }

  if (!info["properties"].toArray().empty()) {
    client->comment("  // properties");
    for (ArrayIter iter(info["properties"]); iter; ++iter) {
      Array prop = iter.second().toArray();
      client->help("  %s%s %s$%s;",
                   prop["doc"].toBoolean() ? "[doc] " : "",
                   prop["access"].toString().data(),
                   GetModifier(prop, "static").data(),
                   prop["name"].toString().data());
    }
  }

  if (!info["methods"].toArray().empty()) {
    client->comment("  // methods");
    for (ArrayIter iter(info["methods"]); iter; ++iter) {
      Array func = iter.second().toArray();
      client->help("  %s%s %s%s%sfunction %s%s(%s);",
                   func["doc"].toBoolean() ? "[doc] " : "",
                   func["access"].toString().data(),
                   GetModifier(func, "static").data(),
                   GetModifier(func, "final").data(),
                   GetModifier(func, "abstract").data(),
                   func["ref"].toBoolean() ? "&" : "",
                   func["name"].toString().data(),
                   GetParams(func["params"]).data());
    }
  }

  client->help("}");
}

///////////////////////////////////////////////////////////////////////////////
}}

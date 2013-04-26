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

#include <runtime/eval/debugger/cmd/cmd_info.h>
#include <runtime/eval/debugger/cmd/cmd_variable.h>
#include <runtime/ext/ext_reflection.h>
#include <runtime/base/preg.h>
#include <util/logger.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

static const StaticString s_params("params");
static const StaticString s_ref("ref");
static const StaticString s_name("name");
static const StaticString s_varg("varg");
static const StaticString s_type("type");
static const StaticString s_default("default");
static const StaticString s_defaultext("defaultext");
static const StaticString s_msg("msg");
static const StaticString s_constants("constants");
static const StaticString s_methods("methods");
static const StaticString s_access("access");
static const StaticString s_static("static");
static const StaticString s_abstract("abstract");
static const StaticString s_final("final");
static const StaticString s_doc("doc");
static const StaticString s_internal("internal");
static const StaticString s_file("file");
static const StaticString s_line1("line1");
static const StaticString s_line2("line2");
static const StaticString s_properties("properties");
static const StaticString s_private_properties("private_properties");
static const StaticString s_defaultText("defaultText");

void CmdInfo::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_type);
  thrift.write(m_symbol);
  thrift.write(m_info);
  if (m_acLiveLists) {
    thrift.write(true);
    thrift.write((int8_t)DebuggerClient::AutoCompleteCount);
    for (int i = 0; i < DebuggerClient::AutoCompleteCount; i++) {
      thrift.write((*m_acLiveLists)[i]);
    }
  } else {
    thrift.write(false);
  }
}

void CmdInfo::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_type);
  thrift.read(m_symbol);
  thrift.read(m_info);
  bool hasLists;
  thrift.read(hasLists);
  if (hasLists) {
    m_acLiveLists = DebuggerClient::CreateNewLiveLists();
    int8_t count;
    thrift.read(count);
    for (int i = 0; i < count; i++) {
      if (i < DebuggerClient::AutoCompleteCount) {
        thrift.read((*m_acLiveLists)[i]);
      } else {
        vector<std::string> future;
        thrift.read(future);
      }
    }
  }
}

void CmdInfo::list(DebuggerClient *client) {
  client->addCompletion(DebuggerClient::AutoCompleteFunctions);
  client->addCompletion(DebuggerClient::AutoCompleteClasses);
  client->addCompletion(DebuggerClient::AutoCompleteClassMethods);
  client->addCompletion(DebuggerClient::AutoCompleteClassProperties);
  client->addCompletion(DebuggerClient::AutoCompleteClassConstants);
}

bool CmdInfo::help(DebuggerClient *client) {
  client->helpTitle("Info Command");
  client->helpCmds(
    "info",                   "displays current function's info",
    "info {cls}",             "displays declaration of this class",
    "info {function}",        "displays declaration of this function",
    "info {cls::method}",     "displays declaration of this method",
    "info {cls::constant}",   "displays declaration of this constant",
    "info {cls::$property}",  "displays declaration of this property",
    nullptr
  );
  client->helpBody(
    "Use this command to display declaration of a symbol."
  );
  return true;
}

bool CmdInfo::parseZeroArg(DebuggerClient *client) {
  assert(client->argCount() == 0);
  BreakPointInfoPtr bpi = client->getCurrentLocation();
  if (bpi) {
    m_symbol = bpi->getClass();
    m_type = KindOfClass;
    if (m_symbol.empty()) {
      m_symbol = bpi->getFunction();
      m_type = KindOfFunction;
    }
  }
  return !m_symbol.empty();
}

void CmdInfo::parseOneArg(DebuggerClient *client, string &subsymbol) {
  assert(client->argCount() == 1);
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
}

bool CmdInfo::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

  string subsymbol;

  if (client->argCount() == 0) {
    if (!parseZeroArg(client)) {
      client->error("There is no current function or method to look up.");
      client->tutorial(
        "You can only use '[i]nfo' without a symbol name when you are running "
        "your program and it breaks at a function or a class method. It will "
        "then look up information about that function or method."
      );
      return true;
    }
  } else if (client->argCount() == 1) {
    parseOneArg(client, subsymbol);
  } else {
    return help(client);
  }

  CmdInfoPtr cmd = client->xend<CmdInfo>(this);
  Array info = cmd->m_info;
  if (info.empty()) {
    client->info("(specified symbol cannot be found)");
  } else {
    for (ArrayIter iter(info); iter; ++iter) {
      StringBuffer sb;
      PrintInfo(client, sb, iter.second(), subsymbol);
      client->code(sb.detach());
    }
  }
  return true;
}

void CmdInfo::UpdateLiveLists(DebuggerClient *client) {
  CmdInfo cmd;
  cmd.m_type = KindOfLiveLists;
  CmdInfoPtr res = client->xend<CmdInfo>(&cmd);
  client->setLiveLists(res->m_acLiveLists);
}

String CmdInfo::GetProtoType(DebuggerClient *client, const std::string &cls,
                             const std::string &func) {
  CmdInfo cmd;
  cmd.m_type = KindOfFunction;
  if (cls.empty()) {
    cmd.m_symbol = String(func);
  } else {
    cmd.m_symbol = String(cls) + "::" + String(func);
  }
  CmdInfoPtr res = client->xend<CmdInfo>(&cmd);
  Array info = res->m_info;
  if (!info.empty()) {
    info = info[0];
    if (info.exists(s_params)) {
      StringBuffer sb;
      sb.printf("function %s%s(%s);\n",
                info[s_ref].toBoolean() ? "&" : "",
                info[s_name].toString().data(),
                GetParams(info[s_params], info[s_varg]).data());
      return sb.detach();
    }
  }
  return String();
}

bool CmdInfo::onServer(DebuggerProxy *proxy) {
  if (m_type == KindOfLiveLists) {
    std::vector<String> tmpAcLiveLists[DebuggerClient::AutoCompleteCount];
    m_acLiveLists = DebuggerClient::CreateNewLiveLists();

    try {
      ClassInfo::GetSymbolNames(
        tmpAcLiveLists[DebuggerClient::AutoCompleteClasses],
        tmpAcLiveLists[DebuggerClient::AutoCompleteFunctions],
        tmpAcLiveLists[DebuggerClient::AutoCompleteConstants],
        &tmpAcLiveLists[DebuggerClient::AutoCompleteClassMethods],
        &tmpAcLiveLists[DebuggerClient::AutoCompleteClassProperties],
        &tmpAcLiveLists[DebuggerClient::AutoCompleteClassConstants]);
    } catch (Exception &e) {
      Logger::Error("Caught exception %s, auto-complete lists incomplete",
                    e.getMessage().c_str());
    } catch(...) {
      Logger::Error("Caught unknown exception, auto-complete lists incomplete");
    }

    int tempList[] = {DebuggerClient::AutoCompleteClasses,
                      DebuggerClient::AutoCompleteFunctions,
                      DebuggerClient::AutoCompleteConstants,
                      DebuggerClient::AutoCompleteClassMethods,
                      DebuggerClient::AutoCompleteClassProperties,
                      DebuggerClient::AutoCompleteClassConstants};

    for (unsigned int i = 0 ; i < sizeof(tempList)/sizeof(int); ++i) {
      for (unsigned int j = 0 ; j < tmpAcLiveLists[tempList[i]].size(); ++j) {
        (*m_acLiveLists)[tempList[i]].push_back(
          tmpAcLiveLists[tempList[i]][j]->toCPPString());
      }
    }

    Array variables = g_vmContext->getLocalDefinedVariables(0);
    variables += CmdVariable::GetGlobalVariables();
    vector<std::string> &vars =
      (*m_acLiveLists)[DebuggerClient::AutoCompleteVariables];
    vars.reserve(variables.size());
    for (ArrayIter iter(variables); iter; ++iter) {
      vars.push_back("$" + iter.first().toString()->toCPPString());
    }

    return proxy->sendToClient(this);
  }

  if (m_type == KindOfUnknown || m_type == KindOfClass) {
    try {
      Array ret = f_hphp_get_class_info(m_symbol);
      if (!ret.empty()) {
        m_info.append(ret);
      }
    } catch (...) {}
  }
  if (m_type == KindOfUnknown || m_type == KindOfFunction) {
    try {
      Array ret = f_hphp_get_function_info(m_symbol);
      if (!ret.empty()) {
        m_info.append(ret);
      }
    } catch (...) {}
  }
  return proxy->sendToClient(this);
}

///////////////////////////////////////////////////////////////////////////////

void CmdInfo::PrintDocComments(StringBuffer &sb, CArrRef info) {
  if (info[s_doc].isString()) {
    String doc = info[s_doc].toString();
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
    sb.printf("%s%s\n", spaces.data(), doc.data());
  }
}

void CmdInfo::PrintHeader(DebuggerClient *client, StringBuffer &sb,
                          CArrRef info) {
  if (!info[s_internal].toBoolean()) {
    String file = info[s_file].toString();
    int line1 = info[s_line1].toInt32();
    int line2 = info[s_line2].toInt32();
    if (file.empty() && line1 == 0 && line2 == 0) {
      sb.printf("// (source unknown)\n");
    } else if (line1 == 0 && line2 == 0) {
      sb.printf("// defined in %s\n", file.data());
    } else if (line1 && line2 && line1 != line2) {
      sb.printf("// defined on line %d to %d of %s\n", line1, line2,
                file.data());
      client->setListLocation(file.data(), line1 - 1, false);
    } else {
      int line = line1 ? line1 : line2;
      sb.printf("// defined on line %d of %s\n", line, file.data());
      client->setListLocation(file.data(), line - 1, false);
    }
  }

  PrintDocComments(sb, info);
}

String CmdInfo::GetParams(CArrRef params, bool varg,
                          bool detailed /* = false */) {
  StringBuffer args;
  for (ArrayIter iter(params); iter; ++iter) {
    if (!args.empty()) {
      args.append(", ");
    }
    Array arg = iter.second().toArray();
    if (!arg[s_type].toString().empty()) {
      args.append(arg[s_type].toString());
      args.append(' ');
    }
    if (arg[s_ref].toBoolean()) {
      args.append('&');
    }
    args.append('$');
    args.append(arg[s_name].toString());
    if (arg.exists(s_default)) {
      args.append(" = ");
      Variant defValue = arg[s_default];
      String defText = arg[s_defaultText];
      if (!defText.empty()) {
        args.append(defText);
      } else if (defValue.isObject()) {
        // ClassInfo was not able to serialize the value, so ext_reflection
        // prepared a stdClass error object. We should fall back to display
        // the original PHP text, if there.
        args.append(defValue.o_get(s_msg).toString());
      } else if (detailed) {
        args.append(DebuggerClient::FormatVariable(arg[s_default], -1));
      } else {
        args.append(DebuggerClient::FormatVariable(arg[s_default]));
      }
    }
  }
  if (varg) {
    if (!args.empty()) {
      args.append(", ");
    }
    args.append("...");
  }
  return args.detach();
}

String CmdInfo::GetModifier(CArrRef info, CStrRef name) {
  if (info[name].toBoolean()) {
    return name + " ";
  }
  return empty_string;
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

bool CmdInfo::TryConstant(StringBuffer &sb, CArrRef info,
                          const std::string &subsymbol) {
  String key = FindSubSymbol(info[s_constants], subsymbol);
  if (!key.isNull()) {
    sb.printf("  const %s = %s;\n", key.data(),
              DebuggerClient::FormatVariable
              (info[s_constants][key], -1).data());
    return true;
  }
  return false;
}

bool CmdInfo::TryProperty(StringBuffer &sb, CArrRef info,
                          const std::string &subsymbol) {
  String key = FindSubSymbol(info[s_properties],
                             subsymbol[0] == '$' ?
                             subsymbol.substr(1) : subsymbol);
  if (!key.isNull()) {
    Array prop = info[s_properties][key];
    PrintDocComments(sb, prop);
    sb.printf("  %s %s$%s;\n",
              prop[s_access].toString().data(),
              GetModifier(prop, s_static).data(),
              prop[s_name].toString().data());
    return true;
  }
  key = FindSubSymbol(info[s_private_properties],
                      subsymbol[0] == '$' ?
                      subsymbol.substr(1) : subsymbol);
  if (!key.isNull()) {
    Array prop = info[s_private_properties][key];
    PrintDocComments(sb, prop);
    sb.printf("  private %s$%s;\n",
              GetModifier(prop, s_static).data(),
              prop[s_name].toString().data());
    return true;
  }
  return false;
}

bool CmdInfo::TryMethod(DebuggerClient *client, StringBuffer &sb, CArrRef info,
                        std::string subsymbol) {
  if (subsymbol.size() > 2 && subsymbol.substr(subsymbol.size() - 2) == "()") {
    subsymbol = subsymbol.substr(0, subsymbol.size() - 2);
  }

  String key = FindSubSymbol(info["methods"], subsymbol);
  if (!key.isNull()) {
    Array func = info["methods"][key].toArray();
    PrintHeader(client, sb, func);
    sb.printf("%s %s%s%sfunction %s::%s%s(%s);\n",
              func[s_access].toString().data(),
              GetModifier(func, s_static).data(),
              GetModifier(func, s_final).data(),
              GetModifier(func, s_abstract).data(),
              info["name"].toString().data(),
              func["ref"].toBoolean() ? "&" : "",
              func["name"].toString().data(),
              GetParams(func["params"], func["varg"], true).data());
    return true;
  }
  return false;
}

void CmdInfo::PrintInfo(DebuggerClient *client, StringBuffer &sb, CArrRef info,
                        const std::string &subsymbol) {
  if (info.exists(s_params)) {
    PrintHeader(client, sb, info);
    sb.printf("function %s%s(%s);\n",
              info[s_ref].toBoolean() ? "&" : "",
              info[s_name].toString().data(),
              GetParams(info[s_params], info[s_varg]).data());
    return;
  }

  bool found = false;
  if (!subsymbol.empty()) {
    if (TryConstant(sb, info, subsymbol)) found = true;
    if (TryProperty(sb, info, subsymbol)) found = true;
    if (TryMethod(client, sb, info, subsymbol)) found = true;
    if (found) return;
    client->info("Specified symbol cannot be found. Here the whole class:\n");
  }

  PrintHeader(client, sb, info);

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

  sb.printf("%s%s%s %s %s{\n",
            GetModifier(info, s_final).data(),
            GetModifier(info, s_abstract).data(),
            info["interface"].toBoolean() ? "interface" : "class",
            info["name"].toString().data(),
            parent.data());

  if (!info["constants"].toArray().empty()) {
    sb.printf("  // constants\n");
    for (ArrayIter iter(info["constants"]); iter; ++iter) {
      sb.printf("  const %s = %s;\n",
                iter.first().toString().data(),
                DebuggerClient::FormatVariable(iter.second()).data());
    }
  }

  if (!info["properties"].toArray().empty() ||
      !info["private_properties"].toArray().empty()) {
    sb.printf("  // properties\n");
    for (ArrayIter iter(info["properties"]); iter; ++iter) {
      Array prop = iter.second().toArray();
      sb.printf("  %s%s %s$%s;\n",
                prop["doc"].toBoolean() ? "[doc] " : "",
                prop["access"].toString().data(),
                GetModifier(prop, s_static).data(),
                prop["name"].toString().data());
    }
    for (ArrayIter iter(info["private_properties"]); iter; ++iter) {
      Array prop = iter.second().toArray();
      sb.printf("  %sprivate %s$%s;\n",
                prop["doc"].toBoolean() ? "[doc] " : "",
                GetModifier(prop, s_static).data(),
                prop["name"].toString().data());
    }
  }

  if (!info["methods"].toArray().empty()) {
    sb.printf("  // methods\n");
    for (ArrayIter iter(info["methods"]); iter; ++iter) {
      Array func = iter.second().toArray();
      sb.printf("  %s%s %s%s%sfunction %s%s(%s);\n",
                func["doc"].toBoolean() ? "[doc] " : "",
                func["access"].toString().data(),
                GetModifier(func, s_static).data(),
                GetModifier(func, s_final).data(),
                GetModifier(func, s_abstract).data(),
                func["ref"].toBoolean() ? "&" : "",
                func["name"].toString().data(),
                GetParams(func["params"], func["varg"]).data());
    }
  }

  sb.printf("}\n");
}

///////////////////////////////////////////////////////////////////////////////
}}

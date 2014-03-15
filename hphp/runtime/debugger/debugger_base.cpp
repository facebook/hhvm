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

#include "hphp/runtime/debugger/debugger_base.h"
#include <utility>
#include <vector>
#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/runtime/debugger/break_point.h"
#include "hphp/parser/scanner.h"
#include "hphp/util/text-util.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

using std::string;

TRACE_SET_MOD(debugger);

const std::string &DSandboxInfo::id() const {
  TRACE(2, "DSandboxInfo::id\n");
  if (m_cached_id.empty() && !m_user.empty()) {
    m_cached_id = m_user + "\t" + m_name;
  }
  return m_cached_id;
}

const std::string DSandboxInfo::desc() const {
  TRACE(2, "DSandboxInfo::desc\n");
  std::string ret = m_user + "'s " + m_name + " sandbox";
  if (!m_path.empty()) {
    ret += " at " + m_path;
  }
  return ret;
}

DSandboxInfo DSandboxInfo::CreateDummyInfo(uint64_t unique) {
  TRACE(2, "DSandboxInfo::CreateDummyInfo\n");
  char buf[64];
  snprintf(buf, 64, "dummy\t0x%" PRIu64, unique);
  return DSandboxInfo(std::string(buf));
}

void DSandboxInfo::set(const std::string &id) {
  TRACE(2, "DSandboxInfo::set\n");
  m_cached_id.clear();
  m_user.clear();
  m_name.clear();
  m_path.clear();
  if (!id.empty()) {
    std::vector<std::string> tokens;
    split('\t', id.c_str(), tokens);
    if (tokens.size() == 2) {
      m_user = tokens[0];
      m_name = tokens[1];
    }
  }
}

void DSandboxInfo::update(const DSandboxInfo &src) {
  TRACE(2, "DSandboxInfo::update\n");
  if (!src.m_path.empty() && m_path.empty()) {
    m_path = src.m_path;
  }
}

void DSandboxInfo::sendImpl(ThriftBuffer &thrift) {
  TRACE(2, "DSandboxInfo::sendImpl\n");
  thrift.write(m_user);
  thrift.write(m_name);
  thrift.write(m_path);
}

void DSandboxInfo::recvImpl(ThriftBuffer &thrift) {
  TRACE(2, "DSandboxInfo::recvImpl\n");
  thrift.read(m_user);
  thrift.read(m_name);
  thrift.read(m_path);
}

///////////////////////////////////////////////////////////////////////////////

void DThreadInfo::sendImpl(ThriftBuffer &thrift) {
  TRACE(2, "DThreadInfo::sendImpl\n");
  thrift.write(m_id);
  thrift.write(m_desc);
  thrift.write(m_type);
  thrift.write(m_url);
}

void DThreadInfo::recvImpl(ThriftBuffer &thrift) {
  TRACE(2, "DThreadInfo::recvImpl\n");
  thrift.read(m_id);
  thrift.read(m_desc);
  thrift.read(m_type);
  thrift.read(m_url);
}

///////////////////////////////////////////////////////////////////////////////

void DFunctionInfo::sendImpl(ThriftBuffer &thrift) {
  TRACE(2, "DFunctionInfo::sendImpl\n");
  thrift.write(m_namespace);
  thrift.write(m_class);
  thrift.write(m_function);
}

void DFunctionInfo::recvImpl(ThriftBuffer &thrift) {
  TRACE(2, "DFunctionInfo::recvImpl\n");
  thrift.read(m_namespace);
  thrift.read(m_class);
  thrift.read(m_function);
}

std::string DFunctionInfo::getName() const {
  TRACE(2, "DFunctionInfo::getName\n");
  if (m_function.empty() || m_class.empty()) {
    return m_function;
  } else {
    return m_class + "::" + m_function;
  }
}

std::string DFunctionInfo::site(std::string &preposition) const {
  TRACE(2, "DFunctionInfo::site\n");
  std::string ret;
  preposition = "at ";
  if (!m_class.empty()) {
    if (!m_namespace.empty()) {
      ret = m_namespace + "\\";
    }
    ret += m_class;
    if (!m_function.empty()) {
      ret += "::" + m_function + "()";
    } else {
      ret = "class " + ret;
      preposition = "in ";
    }
  } else {
    if (!m_function.empty()) {
      ret = m_function + "()";
      if (!m_namespace.empty()) {
        ret += " in namespace " + m_namespace;
      }
    } else if (!m_namespace.empty()) {
      ret = "namespace " + m_namespace;
      preposition = "in ";
    }
  }
  return ret;
}

std::string DFunctionInfo::desc(const BreakPointInfo *bpi) const {
  TRACE(2, "DFunctionInfo::desc\n");
  std::string ret;
  if (!m_class.empty()) {
    string cls;
    if (!m_namespace.empty()) {
      cls = bpi->regex(m_namespace) + "\\";
    }
    cls += bpi->regex(m_class);
    if (!m_function.empty()) {
      ret += cls + "::" + bpi->regex(m_function) + "()";
    } else {
      ret += "any functions in class " + cls;
    }
  } else {
    if (!m_function.empty()) {
      ret += bpi->regex(m_function) + "()";
      if (!m_namespace.empty()) {
        ret += " in namespace " + bpi->regex(m_namespace);
      }
    } else {
      ret += "any functions in namespace " + bpi->regex(m_namespace);
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

std::string Macro::desc(const char *indent) {
  TRACE(2, "Macro::desc\n");
  std::string ret;
  for (unsigned int i = 0; i < m_cmds.size(); i++) {
    if (indent) ret += indent;
    ret += m_cmds[i];
    ret += "\n";
  }
  return ret;
}

void Macro::load(Hdf node) {
  TRACE(2, "Macro::load\n");
  m_name = node["name"].getString();
  node["cmds"].get(m_cmds);
}

void Macro::save(std::ostream &stream, int key) {
  TRACE(2, "Macro::save\n");
  stream << "hhvm.macro.visited." << key << ".name = " << m_name;
  for (unsigned int i = 0; i < m_cmds.size(); i++) {
    stream << "hhvm.macro.visited." << key << ".cmds[" << i << "] = "
           << m_cmds[i];
  }
}

///////////////////////////////////////////////////////////////////////////////

const char *PHP_KEYWORDS[] = {
  "require_once",
  "require",
  "eval",
  "include_once",
  "include",
  "print",
  "instanceof",
  "bool",
  "object",
  "string",
  "double",
  "int",
  "clone",
  "new",
  "exit",
  "if",
  "elseif",
  "else",
  "endif",
  "echo",
  "do",
  "while",
  "endwhile",
  "for",
  "endfor",
  "foreach",
  "endforeach",
  "declare",
  "enddeclare",
  "as",
  "switch",
  "endswitch",
  "case",
  "default",
  "break",
  "continue",
  "function",
  "const",
  "return",
  "try",
  "catch",
  "throw",
  "use",
  "global",
  "public",
  "protected",
  "private",
  "final",
  "abstract",
  "static",
  "var",
  "unset",
  "isset",
  "empty",
  "halt_compiler",
  "class",
  "interface",
  "extends",
  "implements",
  "list",
  "array",
  "__CLASS__",
  "__METHOD__",
  "__FUNCTION__",
  "__LINE__",
  "__FILE__",
  "parent",
  "self",
  nullptr
};

///////////////////////////////////////////////////////////////////////////////

static void get_color(int tokid, int prev, int next,
                      const char *&color, const char *&end,
                      const char **palette =
                      DebuggerClient::DefaultCodeColors) {

  TRACE(7, "debugger_base:get_color\n");
#undef YYTOKENTYPE
#undef YYTOKEN_MAP
#undef YYTOKEN
#define YYTOKEN(num, name) (char)(CodeColorKeyword * 2),
#define YYTOKEN_MAP static char code[] =
#include "hphp/parser/hphp.tab.hpp"
#undef YYTOKEN_MAP
#undef YYTOKEN

#define COLOR_ENTRY(name, type) \
  code[name - YYTOKEN_MIN] = (char)(CodeColor ## type * 2)

  static bool code_inited = false;
  if (!code_inited) {
    code_inited = true;

    COLOR_ENTRY(T_SR_EQUAL,                 None        );
    COLOR_ENTRY(T_SL_EQUAL,                 None        );
    COLOR_ENTRY(T_XOR_EQUAL,                None        );
    COLOR_ENTRY(T_OR_EQUAL,                 None        );
    COLOR_ENTRY(T_AND_EQUAL,                None        );
    COLOR_ENTRY(T_MOD_EQUAL,                None        );
    COLOR_ENTRY(T_CONCAT_EQUAL,             None        );
    COLOR_ENTRY(T_DIV_EQUAL,                None        );
    COLOR_ENTRY(T_MUL_EQUAL,                None        );
    COLOR_ENTRY(T_MINUS_EQUAL,              None        );
    COLOR_ENTRY(T_PLUS_EQUAL,               None        );
    COLOR_ENTRY(T_BOOLEAN_OR,               None        );
    COLOR_ENTRY(T_BOOLEAN_AND,              None        );
    COLOR_ENTRY(T_IS_NOT_IDENTICAL,         None        );
    COLOR_ENTRY(T_IS_IDENTICAL,             None        );
    COLOR_ENTRY(T_IS_NOT_EQUAL,             None        );
    COLOR_ENTRY(T_IS_EQUAL,                 None        );
    COLOR_ENTRY(T_IS_GREATER_OR_EQUAL,      None        );
    COLOR_ENTRY(T_IS_SMALLER_OR_EQUAL,      None        );
    COLOR_ENTRY(T_SR,                       None        );
    COLOR_ENTRY(T_SL,                       None        );
    COLOR_ENTRY(T_DEC,                      None        );
    COLOR_ENTRY(T_INC,                      None        );
    COLOR_ENTRY(T_LNUMBER,                  None        );
    COLOR_ENTRY(T_DNUMBER,                  None        );
    COLOR_ENTRY(T_ONUMBER,                  None        );
    COLOR_ENTRY(T_STRING,                   None        );
    COLOR_ENTRY(T_STRING_VARNAME,           Variable    );
    COLOR_ENTRY(T_VARIABLE,                 Variable    );
    COLOR_ENTRY(T_NUM_STRING,               None        );
    COLOR_ENTRY(T_INLINE_HTML,              Html        );
    COLOR_ENTRY(T_ENCAPSED_AND_WHITESPACE,  String      );
    COLOR_ENTRY(T_CONSTANT_ENCAPSED_STRING, String      );
    COLOR_ENTRY(T_OBJECT_OPERATOR,          None        );
    COLOR_ENTRY(T_DOUBLE_ARROW,             None        );
    COLOR_ENTRY(T_CLASS_C,                  Constant    );
    COLOR_ENTRY(T_METHOD_C,                 Constant    );
    COLOR_ENTRY(T_FUNC_C,                   Constant    );
    COLOR_ENTRY(T_LINE,                     Constant    );
    COLOR_ENTRY(T_FILE,                     Constant    );
    COLOR_ENTRY(T_DIR,                      Constant    );
    COLOR_ENTRY(T_COMMENT,                  Comment     );
    COLOR_ENTRY(T_DOC_COMMENT,              Comment     );
    COLOR_ENTRY(T_OPEN_TAG,                 Tag         );
    COLOR_ENTRY(T_OPEN_TAG_WITH_ECHO,       Tag         );
    COLOR_ENTRY(T_CLOSE_TAG,                Tag         );
    COLOR_ENTRY(T_WHITESPACE,               None        );
    COLOR_ENTRY(T_DOLLAR_OPEN_CURLY_BRACES, None        );
    COLOR_ENTRY(T_CURLY_OPEN,               None        );
    COLOR_ENTRY(T_DOUBLE_COLON,             None        );
  }

  if (tokid == T_STRING) {
    int type = CodeColorConstant;
    if (prev == '$') {
      type = CodeColorVariable;
    } else if (prev == T_FUNCTION || prev == T_CLASS || prev == T_INTERFACE) {
      type = CodeColorDeclaration;
    } else if (next == '(') {
      type = CodeColorNone;
    }
    color = palette[type * 2];
    end = palette[type * 2 + 1];
  } else if (tokid >= YYTOKEN_MIN && tokid <= YYTOKEN_MAX) {
    tokid -= YYTOKEN_MIN;
    char c = code[tokid];
    color = palette[(int)c];
    end = palette[(int)(c+1)];
  } else {
    color = end = nullptr;
  }
}

static void color_line_no(StringBuffer &sb, int line, int lineFocus0,
                          int lineFocus1, const char *color) {
  TRACE(7, "debugger_base:color_line_no\n");
  if (((line == lineFocus0 && lineFocus1 == 0) ||
       (line >= lineFocus0 && line <= lineFocus1)) &&
      DebuggerClient::HighlightBgColor) {
    sb.append(add_bgcolor(DebuggerClient::HighlightForeColor,
                          DebuggerClient::HighlightBgColor));
  } else {
    sb.append(color);
  }
}

static void append_line_no(StringBuffer &sb, const char *text,
                           int &line, const char *color, const char *end,
                           int lineFocus0, int charFocus0, int lineFocus1,
                           int charFocus1, const char **palette =
                           DebuggerClient::DefaultCodeColors) {
  TRACE(7, "debugger_base:append_line_no\n");
  const char *colorLineNo = palette[CodeColorLineNo * 2];
  const char *endLineNo = palette[CodeColorLineNo * 2 + 1];

  // beginning
  if (line && sb.empty()) {
    if (colorLineNo) color_line_no(sb, line, lineFocus0, lineFocus1,
                                   colorLineNo);
    sb.printf(DebuggerClient::LineNoFormat, line);
    if (endLineNo) sb.append(endLineNo);
  }

  // ending
  if (text == nullptr) {
    if (line) {
      if (colorLineNo) color_line_no(sb, line, lineFocus0, lineFocus1,
                                     colorLineNo);
      sb.append("(END)\n");
      if (endLineNo) sb.append(endLineNo);
    }
    return;
  }

  if (color) sb.append(color);

  if (line == 0) {
    sb.append(text);
  } else {
    const char *begin = text;
    const char *p = begin;
    for (; *p; p++) {
      if (*p == '\n') {
        ++line;
        sb.append(begin, p - begin);
        if (color) sb.append(ANSI_COLOR_END);
        sb.append('\n');
        if (colorLineNo) color_line_no(sb, line, lineFocus0, lineFocus1,
                                       colorLineNo);
        if ((line == lineFocus0 && lineFocus1 == 0) ||
            (line >= lineFocus0 && line <= lineFocus1)) {
          sb.printf(DebuggerClient::LineNoFormatWithStar, line);
        } else {
          sb.printf(DebuggerClient::LineNoFormat, line);
        }
        if (endLineNo) sb.append(endLineNo);
        if (color) sb.append(color);
        begin = p + 1;
      }
    }
    if (p - begin > 0) {
      sb.append(begin, p - begin);
    }
  }

  if (end) sb.append(end);
}

String highlight_code(const String& source, int line /* = 0 */,
                      int lineFocus0 /* = 0 */, int charFocus0 /* = 0 */,
                      int lineFocus1 /* = 0 */, int charFocus1 /* = 0 */) {
  TRACE(7, "debugger_base:highlight_code\n");
  String prepended = "<?php\n";
  prepended += source;
  String highlighted = highlight_php(prepended, line, lineFocus0, charFocus0,
                                     lineFocus1, charFocus1);
  int pos = highlighted.find("\n");
  return highlighted.substr(pos + 1);
}

string check_char_highlight(int lineFocus0, int charFocus0,
                            int lineFocus1, int charFocus1,
                            Location &loc) {
  TRACE(7, "debugger_base:check_char_highlight\n");
  if (DebuggerClient::HighlightBgColor &&
      lineFocus0 && charFocus0 && lineFocus1 && charFocus1 &&
      loc.line0 * 1000 + loc.char0 >= lineFocus0 * 1000 + charFocus0 &&
      loc.line1 * 1000 + loc.char1 <= lineFocus1 * 1000 + charFocus1) {
    return add_bgcolor(DebuggerClient::HighlightForeColor,
                       DebuggerClient::HighlightBgColor);
  }
  return "";
}

String highlight_php(const String& source, int line /* = 0 */,
                     int lineFocus0 /* = 0 */, int charFocus0 /* = 0 */,
                     int lineFocus1 /* = 0 */, int charFocus1 /* = 0 */) {
  TRACE(7, "debugger_base:highlight_php\n");
  StringBuffer res;
  Scanner scanner(source.data(), source.size(),
                  Scanner::AllowShortTags | Scanner::ReturnAllTokens);
  ScannerToken tok1, tok2;
  std::vector<std::pair<int, std::string> > ahead_tokens;
  Location loc1, loc2;

  const char *colorComment = nullptr, *endComment = nullptr;
  get_color(T_COMMENT, 0, 0, colorComment, endComment);

  int prev = 0;
  int tokid = scanner.getNextToken(tok1, loc1);
  int next = 0;
  while (tokid) {
    // look ahead
    next = scanner.getNextToken(tok2, loc2);
    while (next == T_WHITESPACE ||
           next == T_COMMENT ||
           next == T_DOC_COMMENT) {

      string text = tok2.text();
      string hcolor = check_char_highlight(lineFocus0, charFocus0,
                                           lineFocus1, charFocus1, loc2);
      if (!hcolor.empty()) {
        text = hcolor + text + ANSI_COLOR_END;
      }

      ahead_tokens.push_back(std::pair<int, string>(next, text));
      next = scanner.getNextToken(tok2, loc2);
    }

    string hcolor = check_char_highlight(lineFocus0, charFocus0,
                                         lineFocus1, charFocus1, loc1);

    if (tokid < 256) {
      if (!hcolor.empty()) {
        res.append(hcolor);
        res.append((char)tokid);
        res.append(ANSI_COLOR_END);
      } else {
        res.append((char)tokid);
      }
    } else {
      const char *color = nullptr, *end = nullptr;
      get_color(tokid, prev, next, color, end);
      if (!hcolor.empty()) {
        color = hcolor.c_str();
        end = ANSI_COLOR_END;
      }

      const std::string &text = tok1.text();
      int offset = 0;
      if (text[0] == '$') {
        if (!hcolor.empty()) {
          res.append(hcolor);
          res.append('$');
          res.append(ANSI_COLOR_END);
        } else {
          res.append('$');
        }
        offset = 1;
      }
      append_line_no(res, text.c_str() + offset, line, color, end,
                     lineFocus0, charFocus0, lineFocus1, charFocus1);
    }

    if (!ahead_tokens.empty()) {
      for (unsigned int i = 0; i < ahead_tokens.size(); i++) {
        bool comment = ahead_tokens[i].first != T_WHITESPACE;
        append_line_no(res, ahead_tokens[i].second.c_str(), line,
                       comment ? colorComment : nullptr,
                       comment ? endComment : nullptr,
                       lineFocus0, charFocus0, lineFocus1, charFocus1);
      }
      ahead_tokens.clear();
    }

    if (!(tokid == T_WHITESPACE || tokid == T_COMMENT ||
          tokid == T_DOC_COMMENT)) {
      prev = tokid;
    }
    tok1 = tok2;
    loc1 = loc2;
    tokid = next;
  }

  append_line_no(res, nullptr, line, nullptr, nullptr,
                 lineFocus0, charFocus0, lineFocus1, charFocus1);
  return res.detach();
}

///////////////////////////////////////////////////////////////////////////////
}}

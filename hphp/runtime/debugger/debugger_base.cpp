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

#include "hphp/runtime/debugger/debugger_base.h"

#include <cstring>
#include <utility>
#include <vector>
#include <boost/algorithm/string/replace.hpp>

#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/runtime/debugger/break_point.h"
#include "hphp/util/text-util.h"
#include "hphp/runtime/base/config.h"

namespace HPHP::Eval {
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
    folly::split('\t', id, tokens);
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

void Macro::load(const IniSetting::Map& ini, Hdf node) {
  TRACE(2, "Macro::load\n");
  Config::Bind(m_name, ini, node["name"]);
  Config::Bind(m_cmds, ini, node["cmds"]);
}

void Macro::save(std::ostream &stream, int key) {
  TRACE(2, "Macro::save\n");
  stream << "hhvm.macros[" << key << "][name] = " << m_name << std::endl;
  for (unsigned int i = 0; i < m_cmds.size(); i++) {
    stream << "hhvm.macros[" << key << "][cmds][" << i << "] = "
           << '"' << boost::replace_all_copy(m_cmds[i], "\"", "\\\"") << '"'
           << std::endl;
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

/**
 * Given a token and a string buffer, append the token to the string buffer, along with
 * color information and line number where applicable. Tokens can span multiple lines and
 * therefore contain a newline char.
*/
static void
append_line_no(StringBuffer& sb, const char* text, int& line, const char* color,
               const char* end, int lineFocus0, int /*charFocus0*/,
               int lineFocus1, int /*charFocus1*/,
               const char** palette = DebuggerClient::DefaultCodeColors) {
  TRACE(7, "debugger_base:append_line_no\n");
  const char *colorLineNo = palette[CodeColorLineNo * 2];
  const char *endLineNo = palette[CodeColorLineNo * 2 + 1];

  // Beginning of the source file
  if (line && sb.empty()) {
    // Add color information to the buffer if line number should be highlighted
    if (colorLineNo) color_line_no(sb, line, lineFocus0, lineFocus1,
                                   colorLineNo);
    sb.printf(DebuggerClient::LineNoFormat, line);
    if (endLineNo) sb.append(endLineNo);
  }

  // End of the source file
  if (text == nullptr) {
    if (line) {
      // Add color information to the buffer if line number should be highlighted
      if (colorLineNo) color_line_no(sb, line, lineFocus0, lineFocus1,
                                     colorLineNo);
      // Do not append a line number, only an END statement
      sb.append("(END)\n");
      if (endLineNo) sb.append(endLineNo);
    }
    return;
  }

  // Add color information to the buffer if token should be highlighted
  if (color) sb.append(color);

  if (line == 0) {
    sb.append(text);
  } else {
    const char *begin = text;
    const char *p = begin;
    for (; *p; p++) {
      /**
      * For any token including a newline, we must handle a new line number. This logic adds
      * the current token, color information for the current line (for both token and line number), newline,
      * line number for the next line, and color information for the next line.
      */
      if (*p == '\n') {
        ++line;
        sb.append(begin, p - begin);
        if (color) sb.append(ANSI_COLOR_END);
        sb.append('\n');
        if (colorLineNo) color_line_no(sb, line, lineFocus0, lineFocus1,
                                       colorLineNo);
        // If the current line is the only focus line or one of multiple focus lines, print the line number with an asterisk
        if ((line == lineFocus0 && lineFocus1 == 0) ||
            (line >= lineFocus0 && line <= lineFocus1)) {
          sb.printf(DebuggerClient::LineNoFormatWithStar, line);
        // If the current line is not a focus line, print the line number by itself
        } else {
          sb.printf(DebuggerClient::LineNoFormat, line);
        }
        if (endLineNo) sb.append(endLineNo);
        if (color) sb.append(color);
        // Advance to the next line
        begin = p + 1;
      }
    } // If token does not span multiple lines, just add it to the buffer
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
  String prepended = "<?hh\n";
  prepended += source;
  String highlighted = highlight_php(prepended, line, lineFocus0, charFocus0,
                                     lineFocus1, charFocus1);
  int pos = highlighted.find("\n");
  return highlighted.substr(pos + 1);
}

/*
 * Given a background color to highlight with, check that the passed in token location
 * resides within the focus area and if so, return a string encoding the background and foreground color.
 */
string check_char_highlight(int lineFocus0, int charFocus0,
                            int lineFocus1, int charFocus1,
                            Location &loc) {
  TRACE(7, "debugger_base:check_char_highlight\n");
  if (DebuggerClient::HighlightBgColor &&
      lineFocus0 && charFocus0 && lineFocus1 && charFocus1 &&
      loc.r.line0 * 1000 + loc.r.char0 >= lineFocus0 * 1000 + charFocus0 &&
      loc.r.line1 * 1000 + loc.r.char1 <= lineFocus1 * 1000 + charFocus1) {
    return add_bgcolor(DebuggerClient::HighlightForeColor,
                       DebuggerClient::HighlightBgColor);
  }
  return "";
}

/*
 * Given a source file, iterate through each line, leaving it up to append_line_no() to write to the string
 * buffer with line number and color information. Add a nullptr termination END line for the last line of
 * the file.
 */
String highlight_php(const String& source, int line /* = 0 */,
                     int lineFocus0 /* = 0 */, int charFocus0 /* = 0 */,
                     int lineFocus1 /* = 0 */, int charFocus1 /* = 0 */) {
  TRACE(7, "debugger_base:highlight_php\n");
  const char *begin = source.data();
  StringBuffer res;
  for (const char *p = begin; *p; p++) {
    if (*p == '\0') {
      break;
    }
    if (*p == '\n') {
      size_t text_len = p - begin + 1;
      // Add space for null terminating character
      size_t total_len = text_len + 2;
      char *text = new char[total_len];
      strncpy(text, begin, text_len);
      text[text_len] = '\0';
      append_line_no(res, text, line, nullptr, nullptr, lineFocus0, charFocus0, lineFocus1, charFocus1);
      begin = p + 1;
    }
  }
  append_line_no(res, nullptr, line, nullptr, nullptr,
                  lineFocus0, charFocus0, lineFocus1, charFocus1);
  return res.detach();
}

///////////////////////////////////////////////////////////////////////////////
}

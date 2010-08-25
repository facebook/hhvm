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

#include <runtime/eval/debugger/debugger_base.h>
#include <runtime/eval/debugger/debugger_client.h>
#include <runtime/eval/parser/scanner.h>
#include <runtime/eval/parser/parser.h>
#include <runtime/eval/debugger/break_point.h>
#include <util/util.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

const std::string &DSandboxInfo::id() const {
  if (m_cached_id.empty() && !m_user.empty()) {
    m_cached_id = m_user + "\t" + m_name;
  }
  return m_cached_id;
}

const std::string DSandboxInfo::desc() const {
  return m_user + "'s " + m_name + " sandbox";
}

void DSandboxInfo::set(const std::string &id) {
  m_cached_id.clear();
  m_user.clear();
  m_name.clear();
  m_path.clear();
  if (!id.empty()) {
    vector<string> tokens;
    Util::split('\t', id.c_str(), tokens);
    if (tokens.size() == 2) {
      m_user = tokens[0];
      m_name = tokens[1];
    }
  }
}

void DSandboxInfo::update(const DSandboxInfo &src) {
  if (!src.m_path.empty() && m_path.empty()) {
    m_path = src.m_path;
  }
}

void DSandboxInfo::sendImpl(ThriftBuffer &thrift) {
  thrift.write(m_user);
  thrift.write(m_name);
  thrift.write(m_path);
}

void DSandboxInfo::recvImpl(ThriftBuffer &thrift) {
  thrift.read(m_user);
  thrift.read(m_name);
  thrift.read(m_path);
}

///////////////////////////////////////////////////////////////////////////////

void DThreadInfo::sendImpl(ThriftBuffer &thrift) {
  thrift.write(m_id);
  thrift.write(m_type);
  thrift.write(m_url);
}

void DThreadInfo::recvImpl(ThriftBuffer &thrift) {
  thrift.read(m_id);
  thrift.read(m_type);
  thrift.read(m_url);
}

///////////////////////////////////////////////////////////////////////////////

void DFunctionInfo::sendImpl(ThriftBuffer &thrift) {
  thrift.write(m_namespace);
  thrift.write(m_class);
  thrift.write(m_function);
}

void DFunctionInfo::recvImpl(ThriftBuffer &thrift) {
  thrift.read(m_namespace);
  thrift.read(m_class);
  thrift.read(m_function);
}

std::string DFunctionInfo::site(std::string &preposition) const {
  string ret;
  preposition = "at ";
  if (!m_class.empty()) {
    if (!m_namespace.empty()) {
      ret = m_namespace + "::";
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
  string ret;
  if (!m_class.empty()) {
    string cls;
    if (!m_namespace.empty()) {
      cls = bpi->regex(m_namespace) + "::";
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
  string ret;
  for (unsigned int i = 0; i < m_cmds.size(); i++) {
    if (indent) ret += indent;
    ret += m_cmds[i];
    ret += "\n";
  }
  return ret;
}

void Macro::load(Hdf node) {
  m_name = node["name"].getString();
  node["cmds"].get(m_cmds);
}

void Macro::save(Hdf node) {
  node["name"] = m_name;
  for (unsigned int i = 0; i < m_cmds.size(); i++) {
    node["cmds"][i] = m_cmds[i];
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
  NULL
};

///////////////////////////////////////////////////////////////////////////////

#define COLOR_ENTRY(number, name, type)  (char)(CodeColor ## type * 2)

static void get_color(int tokid, int prev, int next,
                      const char *&color, const char *&end,
                      const char **palette =
                      DebuggerClient::DefaultCodeColors) {
  static const char code[] = {
    COLOR_ENTRY(258, T_REQUIRE_ONCE,             Keyword     ),
    COLOR_ENTRY(259, T_REQUIRE,                  Keyword     ),
    COLOR_ENTRY(260, T_EVAL,                     Keyword     ),
    COLOR_ENTRY(261, T_INCLUDE_ONCE,             Keyword     ),
    COLOR_ENTRY(262, T_INCLUDE,                  Keyword     ),
    COLOR_ENTRY(263, T_LOGICAL_OR,               Keyword     ),
    COLOR_ENTRY(264, T_LOGICAL_XOR,              Keyword     ),
    COLOR_ENTRY(265, T_LOGICAL_AND,              Keyword     ),
    COLOR_ENTRY(266, T_PRINT,                    Keyword     ),
    COLOR_ENTRY(267, T_SR_EQUAL,                 None        ),
    COLOR_ENTRY(268, T_SL_EQUAL,                 None        ),
    COLOR_ENTRY(269, T_XOR_EQUAL,                None        ),
    COLOR_ENTRY(270, T_OR_EQUAL,                 None        ),
    COLOR_ENTRY(271, T_AND_EQUAL,                None        ),
    COLOR_ENTRY(272, T_MOD_EQUAL,                None        ),
    COLOR_ENTRY(273, T_CONCAT_EQUAL,             None        ),
    COLOR_ENTRY(274, T_DIV_EQUAL,                None        ),
    COLOR_ENTRY(275, T_MUL_EQUAL,                None        ),
    COLOR_ENTRY(276, T_MINUS_EQUAL,              None        ),
    COLOR_ENTRY(277, T_PLUS_EQUAL,               None        ),
    COLOR_ENTRY(278, T_BOOLEAN_OR,               None        ),
    COLOR_ENTRY(279, T_BOOLEAN_AND,              None        ),
    COLOR_ENTRY(280, T_IS_NOT_IDENTICAL,         None        ),
    COLOR_ENTRY(281, T_IS_IDENTICAL,             None        ),
    COLOR_ENTRY(282, T_IS_NOT_EQUAL,             None        ),
    COLOR_ENTRY(283, T_IS_EQUAL,                 None        ),
    COLOR_ENTRY(284, T_IS_GREATER_OR_EQUAL,      None        ),
    COLOR_ENTRY(285, T_IS_SMALLER_OR_EQUAL,      None        ),
    COLOR_ENTRY(286, T_SR,                       None        ),
    COLOR_ENTRY(287, T_SL,                       None        ),
    COLOR_ENTRY(288, T_INSTANCEOF,               Keyword     ),
    COLOR_ENTRY(289, T_UNSET_CAST,               Keyword     ),
    COLOR_ENTRY(290, T_BOOL_CAST,                Keyword     ),
    COLOR_ENTRY(291, T_OBJECT_CAST,              Keyword     ),
    COLOR_ENTRY(292, T_ARRAY_CAST,               Keyword     ),
    COLOR_ENTRY(293, T_STRING_CAST,              Keyword     ),
    COLOR_ENTRY(294, T_DOUBLE_CAST,              Keyword     ),
    COLOR_ENTRY(295, T_INT_CAST,                 Keyword     ),
    COLOR_ENTRY(296, T_DEC,                      None        ),
    COLOR_ENTRY(297, T_INC,                      None        ),
    COLOR_ENTRY(298, T_CLONE,                    Keyword     ),
    COLOR_ENTRY(299, T_NEW,                      Keyword     ),
    COLOR_ENTRY(300, T_EXIT,                     Keyword     ),
    COLOR_ENTRY(301, T_IF,                       Keyword     ),
    COLOR_ENTRY(302, T_ELSEIF,                   Keyword     ),
    COLOR_ENTRY(303, T_ELSE,                     Keyword     ),
    COLOR_ENTRY(304, T_ENDIF,                    Keyword     ),
    COLOR_ENTRY(305, T_LNUMBER,                  None        ),
    COLOR_ENTRY(306, T_DNUMBER,                  None        ),
    COLOR_ENTRY(307, T_STRING,                   None        ),
    COLOR_ENTRY(308, T_STRING_VARNAME,           Variable    ),
    COLOR_ENTRY(309, T_VARIABLE,                 Variable    ),
    COLOR_ENTRY(310, T_NUM_STRING,               None        ),
    COLOR_ENTRY(311, T_INLINE_HTML,              Html        ),
    COLOR_ENTRY(312, T_CHARACTER,                Keyword     ),
    COLOR_ENTRY(313, T_BAD_CHARACTER,            Keyword     ),
    COLOR_ENTRY(314, T_ENCAPSED_AND_WHITESPACE,  String      ),
    COLOR_ENTRY(315, T_CONSTANT_ENCAPSED_STRING, String      ),
    COLOR_ENTRY(316, T_ECHO,                     Keyword     ),
    COLOR_ENTRY(317, T_DO,                       Keyword     ),
    COLOR_ENTRY(318, T_WHILE,                    Keyword     ),
    COLOR_ENTRY(319, T_ENDWHILE,                 Keyword     ),
    COLOR_ENTRY(320, T_FOR,                      Keyword     ),
    COLOR_ENTRY(321, T_ENDFOR,                   Keyword     ),
    COLOR_ENTRY(322, T_FOREACH,                  Keyword     ),
    COLOR_ENTRY(323, T_ENDFOREACH,               Keyword     ),
    COLOR_ENTRY(324, T_DECLARE,                  Keyword     ),
    COLOR_ENTRY(325, T_ENDDECLARE,               Keyword     ),
    COLOR_ENTRY(326, T_AS,                       Keyword     ),
    COLOR_ENTRY(327, T_SWITCH,                   Keyword     ),
    COLOR_ENTRY(328, T_ENDSWITCH,                Keyword     ),
    COLOR_ENTRY(329, T_CASE,                     Keyword     ),
    COLOR_ENTRY(330, T_DEFAULT,                  Keyword     ),
    COLOR_ENTRY(331, T_BREAK,                    Keyword     ),
    COLOR_ENTRY(332, T_CONTINUE,                 Keyword     ),
    COLOR_ENTRY(333, T_FUNCTION,                 Keyword     ),
    COLOR_ENTRY(334, T_CONST,                    Keyword     ),
    COLOR_ENTRY(335, T_RETURN,                   Keyword     ),
    COLOR_ENTRY(336, T_TRY,                      Keyword     ),
    COLOR_ENTRY(337, T_CATCH,                    Keyword     ),
    COLOR_ENTRY(338, T_THROW,                    Keyword     ),
    COLOR_ENTRY(339, T_USE,                      Keyword     ),
    COLOR_ENTRY(340, T_GLOBAL,                   Keyword     ),
    COLOR_ENTRY(341, T_PUBLIC,                   Keyword     ),
    COLOR_ENTRY(342, T_PROTECTED,                Keyword     ),
    COLOR_ENTRY(343, T_PRIVATE,                  Keyword     ),
    COLOR_ENTRY(344, T_FINAL,                    Keyword     ),
    COLOR_ENTRY(345, T_ABSTRACT,                 Keyword     ),
    COLOR_ENTRY(346, T_STATIC,                   Keyword     ),
    COLOR_ENTRY(347, T_VAR,                      Keyword     ),
    COLOR_ENTRY(348, T_UNSET,                    Keyword     ),
    COLOR_ENTRY(349, T_ISSET,                    Keyword     ),
    COLOR_ENTRY(350, T_EMPTY,                    Keyword     ),
    COLOR_ENTRY(351, T_HALT_COMPILER,            Keyword     ),
    COLOR_ENTRY(352, T_CLASS,                    Keyword     ),
    COLOR_ENTRY(353, T_INTERFACE,                Keyword     ),
    COLOR_ENTRY(354, T_EXTENDS,                  Keyword     ),
    COLOR_ENTRY(355, T_IMPLEMENTS,               Keyword     ),
    COLOR_ENTRY(356, T_OBJECT_OPERATOR,          None        ),
    COLOR_ENTRY(357, T_DOUBLE_ARROW,             None        ),
    COLOR_ENTRY(358, T_LIST,                     Keyword     ),
    COLOR_ENTRY(359, T_ARRAY,                    Keyword     ),
    COLOR_ENTRY(360, T_CLASS_C,                  Constant    ),
    COLOR_ENTRY(361, T_METHOD_C,                 Constant    ),
    COLOR_ENTRY(362, T_FUNC_C,                   Constant    ),
    COLOR_ENTRY(363, T_LINE,                     Constant    ),
    COLOR_ENTRY(364, T_FILE,                     Constant    ),
    COLOR_ENTRY(365, T_COMMENT,                  Comment     ),
    COLOR_ENTRY(366, T_DOC_COMMENT,              Comment     ),
    COLOR_ENTRY(367, T_OPEN_TAG,                 Tag         ),
    COLOR_ENTRY(368, T_OPEN_TAG_WITH_ECHO,       Tag         ),
    COLOR_ENTRY(369, T_CLOSE_TAG,                Tag         ),
    COLOR_ENTRY(370, T_WHITESPACE,               None        ),
    COLOR_ENTRY(371, T_START_HEREDOC,            Keyword     ),
    COLOR_ENTRY(372, T_END_HEREDOC,              Keyword     ),
    COLOR_ENTRY(373, T_DOLLAR_OPEN_CURLY_BRACES, None        ),
    COLOR_ENTRY(374, T_CURLY_OPEN,               None        ),
    COLOR_ENTRY(375, T_PAAMAYIM_NEKUDOTAYIM,     None        ),
  };

  if (tokid == 307 /* T_STRING */) {
    int type = CodeColorConstant;
    if (prev == '$') {
      type = CodeColorVariable;
    } else if (prev == 333 /* T_FUNCTION */ ||
               prev == 352 /* T_CLASS */    ||
               prev == 353 /* T_INTERFACE */) {
      type = CodeColorDeclaration;
    } else if (next == '(') {
      type = CodeColorNone;
    }
    color = palette[type * 2];
    end = palette[type * 2 + 1];
  } else if (tokid >= 258 && tokid <= 375) {
    tokid -= 258;
    char c = code[tokid];
    color = palette[(int)c];
    end = palette[(int)(c+1)];
  } else {
    color = end = NULL;
  }
}

static void color_line_no(StringBuffer &sb, int line, int lineFocus0,
                          int lineFocus1, const char *color) {
  if (((line == lineFocus0 && lineFocus1 == 0) ||
       (line >= lineFocus0 && line <= lineFocus1)) &&
      DebuggerClient::HighlightBgColor) {
    sb.append(Util::add_bgcolor(DebuggerClient::HighlightForeColor,
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
  if (text == NULL) {
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
        sb.append(ANSI_COLOR_END);
        sb.append('\n');
        if (colorLineNo) color_line_no(sb, line, lineFocus0, lineFocus1,
                                       colorLineNo);
        sb.printf(DebuggerClient::LineNoFormat, line);
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

String highlight_code(CStrRef source, int line /* = 0 */,
                      int lineFocus0 /* = 0 */, int charFocus0 /* = 0 */,
                      int lineFocus1 /* = 0 */, int charFocus1 /* = 0 */) {
  String prepended = "<?php\n";
  prepended += source;
  String highlighted = highlight_php(prepended, line, lineFocus0, charFocus0,
                                     lineFocus1, charFocus1);
  int pos = highlighted.find("\n");
  return highlighted.substr(pos + 1);
}

string check_char_highlight(int lineFocus0, int charFocus0,
                            int lineFocus1, int charFocus1,
                            ylmm::basic_location loc) {
  if (DebuggerClient::HighlightBgColor &&
      lineFocus0 && charFocus0 && lineFocus1 && charFocus1 &&
      loc.first_line() * 1000 + loc.first_column()
      >= lineFocus0 * 1000 + charFocus0 &&
      loc.last_line() * 1000 + loc.last_column()
      <= lineFocus1 * 1000 + charFocus1) {
    return Util::add_bgcolor(DebuggerClient::HighlightForeColor,
                             DebuggerClient::HighlightBgColor);
  }
  return "";
}

String highlight_php(CStrRef source, int line /* = 0 */,
                     int lineFocus0 /* = 0 */, int charFocus0 /* = 0 */,
                     int lineFocus1 /* = 0 */, int charFocus1 /* = 0 */) {
  Lock lock(Eval::Parser::s_lock);

  const char *input = source.data();
  istringstream iss(input);
  StringBuffer res;

  Eval::Scanner scanner(new ylmm::basic_buffer(iss, false, true),
                        true, false, true);
  Eval::Token tok1, tok2;
  std::vector<pair<int, string> > ahead_tokens;
  ylmm::basic_location loc1, loc2;

  const char *colorComment = NULL, *endComment = NULL;
  get_color(365 /* T_COMMENT */, 0, 0, colorComment, endComment);

  int prev = 0;
  int tokid = scanner.getNextToken(tok1, loc1);
  int next = 0;
  while (tokid) {
    // look ahead
    next = scanner.getNextToken(tok2, loc2);
    while (next == 370 /* T_WHITESPACE */ ||
           next == 365 /* T_COMMENT */ ||
           next == 366 /* T_DOC_COMMENT */) {

      string text = tok2.getText();
      string hcolor = check_char_highlight(lineFocus0, charFocus0,
                                           lineFocus1, charFocus1, loc2);
      if (!hcolor.empty()) {
        text = hcolor + text + ANSI_COLOR_END;
      }

      ahead_tokens.push_back(pair<int, string>(next, text));
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
      const char *color = NULL, *end = NULL;
      get_color(tokid, prev, next, color, end);
      if (!hcolor.empty()) {
        color = hcolor.c_str();
        end = ANSI_COLOR_END;
      }

      const std::string &text = tok1.getText();
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
        bool comment = ahead_tokens[i].first != 370 /* T_WHITESPACE */;
        append_line_no(res, ahead_tokens[i].second.c_str(), line,
                       comment ? colorComment : NULL,
                       comment ? endComment : NULL,
                       lineFocus0, charFocus0, lineFocus1, charFocus1);
      }
      ahead_tokens.clear();
    }

    if (!(tokid == 370 /* T_WHITESPACE */ ||
          tokid == 365 /* T_COMMENT */ ||
          tokid == 366 /* T_DOC_COMMENT */)) {
      prev = tokid;
    }
    tok1 = tok2;
    loc1 = loc2;
    tokid = next;
  }

  append_line_no(res, NULL, line, NULL, NULL,
                 lineFocus0, charFocus0, lineFocus1, charFocus1);
  return res.detach();
}

///////////////////////////////////////////////////////////////////////////////
}}

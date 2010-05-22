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

#include <runtime/eval/parser/scanner.h>
#include <runtime/eval/parser/hphp.tab.hpp>
#include <runtime/eval/ast/statement_list_statement.h>
#include <runtime/eval/ast/if_statement.h>
#include <runtime/eval/ast/try_statement.h>
#include <runtime/eval/ast/array_expression.h>
#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/ast/list_assignment_expression.h>
#include <runtime/eval/ast/switch_statement.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/ast/static_statement.h>

using namespace std;
using namespace HPHP;
using namespace HPHP::Eval;

///////////////////////////////////////////////////////////////////////////////

const string Token::s_empty;

TokenPayload::TokenPayload() : m_mode(None) {}

TokenPayload::~TokenPayload() {
  switch (m_mode) {
  case IfBranch: delete m_data.ifBranches; break;
  case CaseStatement: delete m_data.cases; break;
  case Expression: delete m_data.exprs; break;
  case ListElement: delete m_data.listElems; break;
  case ArrayPair: delete m_data.arrayPairs; break;
  case CatchBlock: delete m_data.catches; break;
  case Parameter: delete m_data.params; break;
  case Name: delete m_data.names; break;
  case StaticVariable: delete m_data.staticVars; break;
  case Strings: delete m_data.strings; break;
  default:
    // All good
    break;
  }
}

void TokenPayload::release() {
  delete this;
}

StatementListStatementPtr TokenPayload::getStmtList() const {
  return getStmt<StatementListStatement>();
}

ExpressionPtr &TokenPayload::exp() {
  if (m_mode == None) {
    m_mode = SingleExpression;
  }
  ASSERT(m_mode = SingleExpression);
  return m_exp;
}

StatementPtr &TokenPayload::stmt() {
  if (m_mode == None) {
    m_mode = SingleStatement;
  }
  ASSERT(m_mode = SingleStatement);
  return m_stmt;
}

NamePtr &TokenPayload::name() {
  if (m_mode == None) {
    m_mode = SingleName;
  }
  ASSERT(m_mode = SingleName);
  return m_name;
}

#define GETTER(type, data)                              \
  std::vector<type##Ptr> &TokenPayload::data() {        \
    if (m_mode == None) {                               \
      m_mode = type;                                    \
      m_data.data = new std::vector<type##Ptr>();       \
    }                                                   \
    ASSERT(m_mode == type);                             \
    return *m_data.data;                                \
  }

  GETTER(IfBranch, ifBranches);
  GETTER(Expression, exprs);
  GETTER(CaseStatement, cases);
  GETTER(ListElement, listElems);
  GETTER(ArrayPair, arrayPairs);
  GETTER(CatchBlock, catches);
  GETTER(Parameter, params);
  GETTER(Name, names);
  GETTER(StaticVariable, staticVars);

#undef GETTER

std::vector<String> &TokenPayload::strings() {
  if (m_mode == None) {
    m_mode = Strings;
    m_data.strings = new std::vector<String>();
  }
  ASSERT(m_mode == Strings);
  return *m_data.strings;
}

TokenPayloadPtr &Token::operator->() {
  if (!m_payload) {
    m_payload = TokenPayloadPtr(new TokenPayload());
  }
  return m_payload;
}

Token &Token::operator=(Token &other) {
  num = other.num;
  text = other.text;
  m_payload = other.m_payload;
  return *this;
}

void Token::setText(const char *t) {
  text = boost::shared_ptr<string>(new string(t));
}

void Token::reset() {
  text.reset();
  num = 0;
  m_payload.reset();
}

Scanner::Scanner(ylmm::basic_buffer* buf, bool bShortTags, bool bASPTags,
                 bool full /* = false */)
  : ylmm::basic_scanner<Token>(buf), m_shortTags(bShortTags),
    m_aspTags(bASPTags), m_full(full), m_line(1), m_column(0) {
  _current->auto_increment(true);
  m_messenger.error_stream(m_err);
  m_messenger.message_stream(m_msg);
  messenger(m_messenger);
  _eval_scanner_init();
}

void Scanner::setToken(const char *rawText, int rawLeng,
                       const char *yytext, int yyleng,
                       bool save /* = true */) {
  if (m_full) {
    _token.text = boost::shared_ptr<string>(new string(rawText, rawLeng));
  } else if (save) {
    _token.text = boost::shared_ptr<string>(new string(yytext, yyleng));
  }
  incLoc(rawText, rawLeng);
}

void Scanner::incLoc(const char *yytext, int yyleng) {
  for (int i = 0; i < yyleng; i++) {
    if (yytext[i] == '\n') {
      m_line++;
      m_column = 0;
    } else {
      m_column++;
    }
  }
}

void Scanner::setDocComment(const char *yytext, int yyleng) {
  m_docComment.assign(yytext, yyleng);
}

void Scanner::setHeredocLabel(const char *label, int len) {
  m_heredocLabel.assign(label, len);
}

int Scanner::getHeredocLabelLen() const {
  return m_heredocLabel.length();
}

const char *Scanner::getHeredocLabel() const {
  return m_heredocLabel.data();
}

void Scanner::resetHeredoc() {
  m_heredocLabel.clear();
}

int Scanner::getNextToken(token_type& t, location_type& l) {
  int tokid;
  bool done = false;

  do {
    tokid = next(t);
    switch (tokid) {
    case T_COMMENT:
    case T_DOC_COMMENT:
    case T_OPEN_TAG:
    case T_WHITESPACE:
      break;
    default:
      done = true;
      break;
    }
  } while (!done && !m_full);

  l.last_line(m_line);
  l.last_column(m_column);
  return tokid;
}

string Scanner::scanEscapeString(char *str, int len, char quote_type) const {
  string output;
  output.reserve(len);

  if (quote_type == '\'') {
    for (int i = 0; i < len; i++) {
      unsigned char ch = str[i];
      if (ch == '\\') {
        if (++i < len) {
          switch (str[i]) {
          case '\\': output += "\\"; break;
          case '\'': output += '\''; break;
          default:
            output += ch;
            output += str[i];
            break;
          }
        } else {
          ASSERT(false);
          output += ch;
        }
      }
      else {
        output += ch;
      }
    }
  } else {
    for (int i = 0; i < len; i++) {
      unsigned char ch = str[i];
      if (ch == '\\') {
        if (++i < len) {
          switch (str[i]) {
          case 'n': output += '\n'; break;
          case 't': output += '\t'; break;
          case 'r': output += '\r'; break;
          case 'v': output += '\v'; break;
          case 'f': output += '\f'; break;
          case '\\': output += '\\'; break;
          case '$': output += '$'; break;
          case '"':
            if (str[i] != quote_type) {
              output += '\\';
              output += '"';
              break;
            } else {
              output += '"';
              break;
            }
          case 'x':
          case 'X':
            if (isxdigit(str[i+1])) {
              string shex;
              shex += str[++i]; // 0th hex digit
              if (isxdigit(str[i+1])) {
                shex += str[++i]; // 1st hex digit
              }
              output += strtol(shex.c_str(), NULL, 16);
            } else {
              output += ch;
              output += str[i];
            }
            break;
          default:
            // check for an octal
            if ('0' <= str[i] && str[i] <= '7') {
              string soct;
              soct += str[i]; // 0th octal digit
              if ('0' <= str[i+1] && str[i+1] <= '7') {
                soct += str[++i];   // 1st octal digit
                if ('0' <= str[i+1] && str[i+1] <= '7') {
                  soct += str[++i]; // 2nd octal digit
                }
              }
              output += strtol(soct.c_str(), NULL, 8);
            } else {
              output += ch;
              output += str[i];
            }
            break;
          }
        } else {
          output += ch;
        }
      } else {
        output += ch;
      }
    }
  }
  return output;
}

void Scanner::flushFlex() {
  _eval_scanner_reset();
}

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

#include <compiler/parser/scanner.h>
#include <compiler/parser/hphp.tab.hpp>

using namespace std;
using namespace boost;
using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// Token

const std::string Token::s_empty;

void Token::setTexts(const char *rawtext, int rawleng,
                     const char *yytext, int yyleng) {
  tokenText = shared_ptr<string>(new string(yytext, yyleng));
  if (rawtext == yytext && rawleng == yyleng) {
    rawText = tokenText;
  } else {
    rawText = shared_ptr<string>(new string(rawtext, rawleng));
  }
}

void Token::setText(const char *text) {
  tokenText = shared_ptr<string>(new string(text));
}

///////////////////////////////////////////////////////////////////////////////
// Scanner

Scanner::Scanner(ylmm::basic_buffer* buf, bool bShortTags, bool bASPTags)
  : ylmm::basic_scanner<Token>(buf), m_shortTags(bShortTags),
    m_aspTags(bASPTags), m_line(1), m_column(0) {
  _current->auto_increment(true);
  m_messenger.error_stream(m_err);
  m_messenger.message_stream(m_msg);
  messenger(m_messenger);
  _scanner_init();
}

void Scanner::setToken(const char *rawText, int rawLeng,
                       const char *yytext, int yyleng) {
  _token.setTexts(rawText,rawLeng, yytext, yyleng);
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

int Scanner::getNextToken(token_type& t, location_type& l)
{
  int tokid;
  bool done = false;

  do {
    tokid = next(t);
    if (tokid != 0) { // not EOF
      for (const char *s = t.rawtext().c_str(); *s; s++) {
        if (*s == '\n') {
          m_line++;
          m_column = 0;
        } else {
          m_column++;
        }
      }
    }
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
  } while (!done);

  l.last_line(m_line);
  l.last_column(m_column);
  return tokid;
}

string Scanner::scanEscapeString(char *input, int len, char quote_type) const {
  string output;
  string str(input, len);

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


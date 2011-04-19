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

#include "scanner.h"
#include <util/util.h>
#include <util/preprocess.h>
#include <util/logger.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void ScannerToken::xhpLabel(bool prefix /* = true */) {
  Util::replaceAll(m_text, ":", "__");
  Util::replaceAll(m_text, "-", "_");
  if (prefix) {
    m_text = "xhp_" + m_text;
  }
}

bool ScannerToken::htmlTrim() {
  ASSERT(!m_text.empty());

  const char *p0 = m_text.c_str();
  const char *p1 = m_text.c_str() + m_text.size() - 1;
  const char *p00 = p0;
  const char *p10 = p1;
  while (isspace(*p0) && p0 <= p10) ++p0;
  if (p0 > p10) {
    m_text.clear();
    return false;
  }
  while (isspace(*p1) && p1 > p0) --p1;
  string text;
  text.reserve(m_text.length());
  if (p0 != p00) {
    text = " ";
  }
  for (const char *p = p0; p <= p1; ++p) {
    if (!isspace(*p)) {
      text += *p;
    } else {
      while (isspace(*p)) ++p;
      text += ' ';
      text += *p;
    }
  }
  if (p1 != p10) {
    text += " ";
  }
  m_text = text;
  return true;
}

// hzhao: This is to avoid including headers from runtime/base.
extern char *string_html_decode(const char *input, int &len,
                                const char *charset_hint, bool all,
                                bool xhp);

void ScannerToken::xhpDecode() {
  int len = m_text.size();
  char *ret = string_html_decode(m_text.data(), len, "UTF-8", true, true);
  m_text = string(ret, len);
  free(ret);
}

///////////////////////////////////////////////////////////////////////////////

Scanner::Scanner(const char *filename, int type)
    : m_filename(filename), m_source(NULL), m_len(0), m_pos(0),
      m_state(Start), m_type(type), m_yyscanner(NULL), m_token(NULL),
      m_loc(NULL), m_lastToken(-1), m_gap(false), m_inScript(false),
      m_xhpState(0) {
  m_stream = new ifstream(filename);
  m_streamOwner = true;
  if (m_stream->bad()) {
    delete m_stream; m_stream = NULL;
    throw FileOpenException(filename);
  }
  if (type & PreprocessXHP) {
    istream *is = preprocessXHP(*m_stream, m_sstream, filename);
    if (m_stream != is) {
      delete m_stream;
      m_stream = is;
      m_streamOwner = false;
    }
  }
  init();
}

Scanner::Scanner(istream &stream, int type, const char *fileName /* = "" */)
    : m_filename(fileName), m_source(NULL), m_len(0), m_pos(0),
      m_state(Start), m_type(type), m_yyscanner(NULL), m_token(NULL),
      m_loc(NULL), m_lastToken(-1), m_gap(false), m_inScript(false),
      m_xhpState(0) {
  m_stream = &stream;
  m_streamOwner = false;
  if (type & PreprocessXHP) {
    istream *is = preprocessXHP(*m_stream, m_sstream, fileName);
    if (m_stream != is) {
      m_stream = is;
    }
  }
  init();
}

Scanner::Scanner(const char *source, int len, int type,
                 const char *fileName /* = "" */)
    : m_filename(fileName), m_source(source), m_len(len), m_pos(0),
      m_state(Start), m_type(type), m_yyscanner(NULL), m_token(NULL),
      m_loc(NULL), m_lastToken(-1), m_gap(false), m_inScript(false),
      m_xhpState(0) {
  ASSERT(m_source);
  m_stream = NULL;
  m_streamOwner = false;
  if (type & PreprocessXHP) {
    m_stream = new istringstream(string(source, len));
    m_streamOwner = true;
    istream *is = preprocessXHP(*m_stream, m_sstream, fileName);
    if (m_stream != is) {
      delete m_stream;
      m_stream = is;
      m_streamOwner = false;
    }
  }
  init();
}

Scanner::~Scanner() {
  reset();
  if (m_streamOwner) {
    delete m_stream;
  }
}

void Scanner::setHashBang(const char *rawText, int rawLeng) {
  if (m_type & ReturnAllTokens) {
    setToken(rawText, rawLeng);
  } else {
    m_token->setText("", 0);
    incLoc(rawText, rawLeng);
  }
}

int Scanner::getNextToken(ScannerToken &t, Location &l) {
  m_token = &t;
  m_loc = &l;

  int tokid;
  bool done = false;
  m_gap = false;
  do {
    tokid = scan();
    switch (tokid) {
      case T_COMMENT:
      case T_DOC_COMMENT:
      case T_OPEN_TAG:
      case T_WHITESPACE:
        m_gap = true;
        break;
      default:
        done = true;
        break;
    }
  } while (!done && (m_type & ReturnAllTokens) == 0);

  m_lastToken = tokid;
  return tokid;
}

int Scanner::read(char *text, int &result, int max) {
  if (m_stream) {
    if (!m_stream->eof()) {
      m_stream->read(text, max);
      if (!m_stream->bad()) {
        return (result = m_stream->gcount());
      }
    }
  } else if (m_source) {
    if (m_pos < m_len) {
      int count = m_len - m_pos;
      if (count > max) count = max;
      if (count > 0) {
        memcpy(text, m_source + m_pos, count);
        m_pos += count;
        return (result = count);
      }
    }
  }
  return (result = 0);
}

void Scanner::error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  Util::string_vsnprintf(m_error, fmt, ap);
  va_end(ap);
}

void Scanner::warn(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  string msg;
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  Logger::Warning("%s: %s (Line: %d, Char %d)", msg.c_str(),
                  m_filename.c_str(), m_loc->line0, m_loc->char0);
}

void Scanner::incLoc(const char *rawText, int rawLeng) {
  ASSERT(rawText);
  ASSERT(rawLeng > 0);

  switch (m_state) {
    case Start:
      break; // scanner set to (1, 1, 1, 1) already
    case NoLineFeed:
      m_loc->line0 = m_loc->line1;
      m_loc->char0 = m_loc->char1 + 1;
      break;
    case HadLineFeed:
      m_loc->line0 = m_loc->line1 + 1;
      m_loc->char0 = 1;
      break;
  }
  const char *p = rawText;
  for (int i = 0; i < rawLeng; i++) {
    switch (m_state) {
      case Start:
        break; // scanner set to (1, 1, 1, 1) already
      case NoLineFeed:
        m_loc->char1++;
        break;
      case HadLineFeed:
        m_loc->line1++;
        m_loc->char1 = 1;
        break;
    }
    m_state = (*p++ == '\n' ? HadLineFeed : NoLineFeed);
  }
}

string Scanner::escape(char *str, int len, char quote_type) const {
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
            default: {
              output += ch;
              output += str[i];
              break;
            }
          }
        } else {
          ASSERT(false);
          output += ch;
        }
      } else {
        output += ch;
      }
    }
  } else {
    for (int i = 0; i < len; i++) {
      unsigned char ch = str[i];
      if (ch == '\\') {
        if (++i < len) {
          switch (str[i]) {
            case 'n':  output += '\n'; break;
            case 't':  output += '\t'; break;
            case 'r':  output += '\r'; break;
            case 'v':  output += '\v'; break;
            case 'f':  output += '\f'; break;
            case '\\': output += '\\'; break;
            case '$':  output += '$';  break;
            case '"':
              if (str[i] != quote_type) {
                output += '\\';
              }
              output += '"';
              break;
            case 'x':
            case 'X': {
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
            }
            default: {
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

///////////////////////////////////////////////////////////////////////////////
}

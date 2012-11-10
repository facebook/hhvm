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

#include "util/parser/scanner.h"
#include "util/util.h"
#include "util/preprocess.h"
#include "util/logger.h"
#include "util/zend/zend_string.h"

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
                                bool decode_double_quote,
                                bool decode_single_quote,
                                const char *charset_hint, bool all,
                                bool xhp);

void ScannerToken::xhpDecode() {
  int len = m_text.size();
  char *ret = string_html_decode(m_text.c_str(), len, true,
                                 false, "UTF-8", true, true);
  m_text = string(ret, len);
  free(ret);
}

///////////////////////////////////////////////////////////////////////////////

Scanner::Scanner(const char *filename, int type, bool md5 /* = false */)
    : m_filename(filename), m_stream(NULL), m_source(NULL), m_len(0), m_pos(0),
      m_state(Start), m_type(type), m_yyscanner(NULL), m_token(NULL),
      m_loc(NULL), m_lastToken(-1), m_isStrictMode(0), m_lookaheadLtDepth(0) {
  m_stream = new std::ifstream(filename);
  m_streamOwner = true;
  if (m_stream->fail()) {
    delete m_stream; m_stream = NULL;
    throw FileOpenException(filename);
  }
  if (md5) computeMd5();
  if (type & PreprocessXHP) {
    std::istream *is = preprocessXHP(*m_stream, m_sstream, filename);
    if (m_stream != is) {
      delete m_stream;
      m_stream = is;
      m_streamOwner = false;
    }
  }
  init();
}

Scanner::Scanner(std::istream &stream, int type,
                 const char *fileName /* = "" */,
                 bool md5 /* = false */)
    : m_filename(fileName), m_source(NULL), m_len(0), m_pos(0),
      m_state(Start), m_type(type), m_yyscanner(NULL), m_token(NULL),
      m_loc(NULL), m_lastToken(-1), m_isStrictMode(0), m_lookaheadLtDepth(0) {
  m_stream = &stream;
  m_streamOwner = false;
  if (md5) computeMd5();
  if (type & PreprocessXHP) {
    std::istream *is = preprocessXHP(*m_stream, m_sstream, fileName);
    if (m_stream != is) {
      m_stream = is;
    }
  }
  init();
}

Scanner::Scanner(const char *source, int len, int type,
                 const char *fileName /* = "" */, bool md5 /* = false */)
    : m_filename(fileName), m_stream(NULL), m_source(source), m_len(len),
      m_pos(0), m_state(Start), m_type(type), m_yyscanner(NULL),
      m_token(NULL), m_loc(NULL), m_lastToken(-1), m_isStrictMode(0),
      m_lookaheadLtDepth(0) {
  ASSERT(m_source);
  m_streamOwner = false;
  if (md5 || type & PreprocessXHP) {
    m_stream = new std::istringstream(string(source, len));
    m_streamOwner = true;
    if (md5) computeMd5();
    if (type & PreprocessXHP) {
      std::istream *is = preprocessXHP(*m_stream, m_sstream, fileName);
      if (m_stream != is) {
        delete m_stream;
        m_stream = is;
        m_streamOwner = false;
      }
    }
  }

  init();
}

void Scanner::computeMd5() {
  int startpos = m_stream->tellg();
  m_stream->seekg(0, std::ios::end);
  int length = m_stream->tellg();
  m_stream->seekg(0, std::ios::beg);
  char *ptr = (char*)malloc(length);
  m_stream->read(ptr, length);
  m_stream->seekg(startpos, std::ios::beg);
  int out_len;
  char *md5str = string_md5(ptr, length, false, out_len);
  free(ptr);
  m_md5 = string(md5str, out_len);
  free(md5str);
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

// scanToken() will always get a new token from the frontier
// regardless of whether there are tokens in the lookahead store
int Scanner::scanToken(ScannerToken &t, Location &l) {
  m_token = &t;
  m_loc = &l;
  int tokid;
  for (;;) {
    tokid = scan();
    switch (tokid) {
      case T_DOC_COMMENT:
        setDocComment(m_token->text());
        /* fall through */
      case T_COMMENT:
      case T_OPEN_TAG:
      case T_WHITESPACE:
        if (m_type & ReturnAllTokens) {
          // m_lastToken holds the last "signficant" token, so
          // don't update it for comments or whitespace
          return tokid;
        }
        break;
      default:
        m_lastToken = tokid;
        return tokid;
    }
  }
}

// fetchToken() will return the first token in the lookahead store (if the
// lookahead store has tokens) or it will get a new token from the frontier
int Scanner::fetchToken(ScannerToken &t, Location &l) {
  m_token = &t;
  m_loc = &l;
  int tokid;
  if (!m_lookahead.empty()) {
    // If there is a lookahead token, return that. No need to perform
    // special logic for "ReturnAllTokens", we already accounted for
    // that when the tokens were inserted into m_lookahead
    TokenStore::iterator it = m_lookahead.begin();
    tokid = it->t;
    *m_token = it->token;
    *m_loc = it->loc;
    return tokid;
  }
  return scanToken(t,l);
}

// nextLookahead() advances an iterator forward in the lookahead store.
// If the end of the store is reached, a new token will be scanned from
// the frontier. nextLookahead skips over whitespace and comments.
void Scanner::nextLookahead(TokenStore::iterator& pos) {
  for (;;) {
    ++pos;
    if (pos == m_lookahead.end()) {
      pos = m_lookahead.appendNew();
      pos->loc = *m_loc;
      pos->t = scanToken(pos->token, pos->loc);
    }
    switch (pos->t) {
      case T_DOC_COMMENT:
      case T_COMMENT:
      case T_OPEN_TAG:
      case T_WHITESPACE:
        break;
      default:
        return;
    }
  }
}

bool Scanner::tryParseTypeList(TokenStore::iterator& pos) {
  for (;;) {
    if (!tryParseNSType(pos)) return false;
    if (pos->t == T_AS) {
      nextLookahead(pos);
      if (!tryParseNSType(pos)) return false;
    }
    if (pos->t != ',') return true;
    nextLookahead(pos);
  }
}

bool
Scanner::tryParseNSType(TokenStore::iterator& pos) {
  if (pos->t == '@') {
    nextLookahead(pos);
  }
  if (pos->t == '?') {
    nextLookahead(pos);
  }
  if (pos->t == '(') {
    nextLookahead(pos);
    if (pos->t == T_FUNCTION) {
      nextLookahead(pos);
      if (pos->t != '(') return false;
      nextLookahead(pos);
      if (!tryParseTypeList(pos) || pos->t != ')') return false;
      nextLookahead(pos);
      if (pos->t == ')') {
        nextLookahead(pos);
        return true;
      }
      if (pos->t != ':') return false;
      nextLookahead(pos);
      if (!tryParseNSType(pos) || pos->t != ')') return false;
      nextLookahead(pos);
      return true;
    }
    if (!tryParseTypeList(pos) || pos->t != ')') return false;
    nextLookahead(pos);
    return true;
  } 
  if (pos->t == T_NAMESPACE) {
    nextLookahead(pos);
    if (pos->t != T_NS_SEPARATOR) return false;
    nextLookahead(pos);
  } else if (pos->t == T_NS_SEPARATOR) {
    nextLookahead(pos);
  }
  for (;;) {
    switch (pos->t) {
      case T_STRING:
      case T_XHP_ATTRIBUTE:
      case T_XHP_CATEGORY:
      case T_XHP_CHILDREN:
      case T_XHP_REQUIRED:
      case T_XHP_ENUM:
      case T_ARRAY:
        nextLookahead(pos);
        break;
      case T_XHP_LABEL:
        nextLookahead(pos);
        return true; 
      default:
        return false;
    }
    if (pos->t == T_UNRESOLVED_LT) {
      TokenStore::iterator ltPos = pos;
      nextLookahead(pos);
      ++m_lookaheadLtDepth;
      bool isTypeList = tryParseTypeList(pos);
      --m_lookaheadLtDepth;
      if (!isTypeList || pos->t != '>') {
        ltPos->t = '<';
        return false;
      }
      ltPos->t = T_TYPELIST_LT;
      pos->t = T_TYPELIST_GT;
      nextLookahead(pos);
      return true;
    }
    if (pos->t != T_NS_SEPARATOR) {
      return true;
    }
    nextLookahead(pos);
  }
}

int Scanner::getNextToken(ScannerToken &t, Location &l) {
  int tokid;
  bool la = !m_lookahead.empty();
  tokid = fetchToken(t, l);
  if (LIKELY(tokid != T_UNRESOLVED_LT)) {
    // In the common case, we don't have to perform any resolution
    // and we can just return the token
    if (UNLIKELY(la)) {
      // If we pulled a lookahead token, we need to remove it from
      // the lookahead store
      m_lookahead.popFront();
    }
    return tokid;
  }
  // We encountered a '<' character that needs to be resolved.
  if (!la) {
    // If this token didn't come from the lookahead store, we
    // need to stash it there
    TokenStore::iterator it = m_lookahead.appendNew();
    LookaheadToken ltd = { t, l, tokid };
    *it = ltd;
  }
  // Look at subsequent tokens to determine if the '<' character
  // is the start of a type list
  TokenStore::iterator pos = m_lookahead.begin();
  TokenStore::iterator ltPos = pos;
  nextLookahead(pos); 
  ++m_lookaheadLtDepth;
  bool isTypeList = tryParseTypeList(pos);
  --m_lookaheadLtDepth;
  if (!isTypeList || pos->t != '>') {
    ltPos->t = '<';
  } else {
    ltPos->t = T_TYPELIST_LT;
    pos->t = T_TYPELIST_GT;
  }
  tokid = fetchToken(t, l);
  // We pulled a lookahead token, we need to remove it from the
  // lookahead store
  m_lookahead.popFront();
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

TokenStore::iterator TokenStore::begin() {
  if (empty()) {
    return end();
  }
  iterator it;
  it.m_slab = m_head;
  it.m_pos = m_head->m_beginPos;
  return it;
}

TokenStore::iterator TokenStore::end() {
  iterator it;
  it.m_slab = NULL;
  it.m_pos = 0;
  return it;
}

void TokenStore::popFront() {
  if (empty()) return;
  ++m_head->m_beginPos;
  if (m_head->m_beginPos < m_head->m_endPos) return;
  LookaheadSlab* nextSlab = m_head->m_next;
  if (!nextSlab) {
    // We just removed the last token from the last slab. We hang on to the
    // last slab instead of freeing it so that we don't keep allocating and
    // freeing slabs in the common steady state.
    m_head->m_beginPos = 0;
    m_head->m_endPos = 0;
    return;
  }
  delete m_head;
  m_head = nextSlab;
}

TokenStore::iterator TokenStore::appendNew() {
  iterator it;
  if (m_tail && m_tail->m_endPos < LookaheadSlab::SlabSize) {
    it.m_slab = m_tail;
    it.m_pos = m_tail->m_endPos;
    ++m_tail->m_endPos;
    return it;
  }
  LookaheadSlab* newSlab = new LookaheadSlab;
  newSlab->m_next = NULL;
  newSlab->m_beginPos = 0;
  newSlab->m_endPos = 0;
  if (m_tail) {
    m_tail->m_next = newSlab;
    m_tail = m_tail->m_next;
  } else {
    m_head = m_tail = newSlab;
  }
  it.m_slab = m_tail;
  it.m_pos = newSlab->m_endPos;
  ++newSlab->m_endPos;
  return it;
}

///////////////////////////////////////////////////////////////////////////////
}

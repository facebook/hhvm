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
#include "hphp/parser/scanner.h"

#include <fstream>

#include "hphp/util/text-util.h"
#include "hphp/util/logger.h"
#include "hphp/zend/zend-string.h"
#include "hphp/zend/zend-html.h"
#include "hphp/util/string-vsnprintf.h"
#include "hphp/parser/parse-time-fatal-exception.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void ScannerToken::xhpLabel(bool prefix /* = true */) {
  replaceAll(m_text, ":", "__");
  replaceAll(m_text, "-", "_");
  if (prefix) {
    m_text = "xhp_" + m_text;
  }
}

bool ScannerToken::htmlTrim() {
  assert(!m_text.empty());

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
  std::string text;
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

void ScannerToken::xhpDecode() {
  int len = m_text.size();
  // note: 5th arg is charset_hint string; here we pass nullptr to indicate
  // "use the default one" which is UTF-8.  (Just saves a charset lookup.)
  char *ret = string_html_decode(m_text.c_str(), len, true,
                                 false, nullptr, true, true);
  // safety check: decode function returns null iff charset unrecognized;
  // i.e. nullptr result would mean UTF-8 is available.
  // Pretty sure it is universally available!
  // (Do assertion anyway.)
  assert(ret);
  m_text = std::string(ret, len);
  free(ret);
}

///////////////////////////////////////////////////////////////////////////////

Scanner::Scanner(const std::string& filename, int type, bool md5 /* = false */)
    : m_filename(filename), m_stream(nullptr), m_source(nullptr), m_len(0), m_pos(0),
      m_state(Start), m_type(type), m_yyscanner(nullptr), m_token(nullptr),
      m_loc(nullptr), m_lastToken(-1), m_isHHFile(0), m_lookaheadLtDepth(0),
      m_listener(nullptr) {
  m_stream = new std::ifstream(m_filename);
  m_streamOwner = true;
  if (m_stream->fail()) {
    delete m_stream; m_stream = nullptr;
    throw FileOpenException(m_filename);
  }
  if (md5) computeMd5();
  init();
}

Scanner::Scanner(std::istream &stream, int type,
                 const char *fileName /* = "" */,
                 bool md5 /* = false */)
    : m_filename(fileName), m_source(nullptr), m_len(0), m_pos(0),
      m_state(Start), m_type(type), m_yyscanner(nullptr), m_token(nullptr),
      m_loc(nullptr), m_lastToken(-1), m_isHHFile(0), m_lookaheadLtDepth(0),
      m_listener(nullptr) {
  m_stream = &stream;
  m_streamOwner = false;
  if (md5) computeMd5();
  init();
}

Scanner::Scanner(const char *source, int len, int type,
                 const char *fileName /* = "" */, bool md5 /* = false */)
    : m_filename(fileName), m_stream(nullptr), m_source(source), m_len(len),
      m_pos(0), m_state(Start), m_type(type), m_yyscanner(nullptr),
      m_token(nullptr), m_loc(nullptr), m_lastToken(-1), m_isHHFile(0),
      m_lookaheadLtDepth(0), m_listener(nullptr) {
  assert(m_source);
  m_streamOwner = false;
  if (md5) {
    m_stream = new std::istringstream(std::string(source, len));
    m_streamOwner = true;
    computeMd5();
  }

  init();
}

void Scanner::computeMd5() {
  size_t startpos = m_stream->tellg();
  always_assert(startpos != -1 &&
                startpos <= std::numeric_limits<int32_t>::max());
  m_stream->seekg(0, std::ios::end);
  size_t length = m_stream->tellg();
  always_assert(length != -1 &&
                length <= std::numeric_limits<int32_t>::max());
  m_stream->seekg(0, std::ios::beg);
  char *ptr = (char*)malloc(length);
  m_stream->read(ptr, length);
  m_stream->seekg(startpos, std::ios::beg);
  m_md5 = string_md5(ptr, length);
  free(ptr);
}

Scanner::~Scanner() {
  reset();
  if (m_streamOwner) {
    delete m_stream;
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

bool Scanner::nextIfToken(TokenStore::iterator& pos, int tok) {
  if (pos->t != tok) return false;
  nextLookahead(pos);
  return true;
}

bool Scanner::tryParseTypeList(TokenStore::iterator& pos) {
  for (;;) {
    if (pos->t == '+' || pos->t == '-') {
      nextLookahead(pos);
    }
    if (!tryParseNSType(pos)) return false;
    if (pos->t == T_AS || pos->t == T_SUPER) {
      nextLookahead(pos);
      if (!tryParseNSType(pos)) {
        return false;
      }
    }
    if (pos->t != ',') return true;
    nextLookahead(pos);
  }
}

bool Scanner::tryParseNonEmptyLambdaParams(TokenStore::iterator& pos) {
  for (;; nextLookahead(pos)) {
    if (pos->t == ')' || pos->t == T_LAMBDA_CP) return true;
    if (pos->t != T_VARIABLE) {
      if (pos->t == T_ELLIPSIS) {
        nextLookahead(pos);
        return true;
      }
      if (!tryParseNSType(pos)) return false;
      if (pos->t == '&') {
        nextLookahead(pos);
      }
      if (pos->t != T_VARIABLE) return false;
    }
    nextLookahead(pos);
    if (pos->t == '=') {
      nextLookahead(pos);
      parseApproxParamDefVal(pos);
    }
    if (pos->t != ',') return true;
  }
}

void Scanner::parseApproxParamDefVal(TokenStore::iterator& pos) {
  int64_t opNum = 0; // counts nesting for ( and T_UNRESOLVED_OP
  int64_t obNum = 0; // counts nesting for [
  int64_t ocNum = 0; // counts nesting for {
  int64_t ltNum = 0; // counts nesting for T_TYPELIST_LT
  for (;; nextLookahead(pos)) {
    switch (pos->t) {
      case ',':
        if (!opNum && !obNum && !ocNum && !ltNum) return;
        break;
      case '(':
      case T_UNRESOLVED_OP:
        ++opNum;
        break;
      case ')':
        if (!opNum) return;
        --opNum;
        break;
      case '[':
        ++obNum;
        break;
      case ']':
        if (!obNum) return;
        --obNum;
        break;
      case '{':
        ++ocNum;
        break;
      case '}':
        if (!ocNum) return;
        --ocNum;
        break;
      case T_TYPELIST_LT:
        ++ltNum;
        break;
      case T_UNRESOLVED_LT: {
        auto endPos = pos;
        nextLookahead(endPos);
        if (tryParseTypeList(endPos) && endPos->t == '>') {
          pos->t = T_TYPELIST_LT;
          endPos->t = T_TYPELIST_GT;
        } else {
          pos->t = '<';
        }
        ++ltNum;
        break;
      }
      case T_TYPELIST_GT:
        if (!ltNum) return;
        --ltNum;
        break;
      case T_LNUMBER:
      case T_DNUMBER:
      case T_ONUMBER:
      case T_CONSTANT_ENCAPSED_STRING:
      case T_START_HEREDOC:
      case T_ENCAPSED_AND_WHITESPACE:
      case T_END_HEREDOC:
      case T_LINE:
      case T_FILE:
      case T_DIR:
      case T_CLASS_C:
      case T_TRAIT_C:
      case T_METHOD_C:
      case T_FUNC_C:
      case T_NS_C:
      case T_COMPILER_HALT_OFFSET:
      case T_STRING:
      case T_ENUM:
      case T_XHP_LABEL:
      case T_XHP_ATTRIBUTE:
      case T_XHP_CATEGORY:
      case T_XHP_CHILDREN:
      case T_XHP_REQUIRED:
      case T_NS_SEPARATOR:
      case T_NAMESPACE:
      case T_SHAPE:
      case T_ARRAY:
      case T_FUNCTION:
      case T_DOUBLE_ARROW:
      case T_DOUBLE_COLON:
      case '+':
      case '-':
      case ':':
      case '?':
      case '@':
        break;
      default:
        return;
    }
  }
}

bool Scanner::tryParseFuncTypeList(TokenStore::iterator& pos) {
  for (;;) {
    if (pos->t == T_ELLIPSIS) {
      nextLookahead(pos);
      return true;
    }
    if (!tryParseNSType(pos)) return false;
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
  if (pos->t == '(' || pos->t == T_UNRESOLVED_OP) {
    nextLookahead(pos);
    if (pos->t == T_FUNCTION) {
      nextLookahead(pos);
      if (pos->t != '(') return false;
      nextLookahead(pos);
      if (pos->t != ')') {
        if (!tryParseFuncTypeList(pos)) return false;
        if (pos->t != ')') return false;
      }
      nextLookahead(pos);
      if (pos->t == ')') {
        nextLookahead(pos);
        return true;
      }
      if (pos->t != ':') return false;
      nextLookahead(pos);
      if (!tryParseNSType(pos)) return false;
      if (pos->t != ')') return false;
      nextLookahead(pos);
      return true;
    }
    if (!tryParseTypeList(pos)) return false;
    if (pos->t != ')') return false;
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
      case T_SUPER:
      case T_XHP_ATTRIBUTE:
      case T_XHP_CATEGORY:
      case T_XHP_CHILDREN:
      case T_XHP_REQUIRED:
      case T_ENUM:
      case T_ARRAY:
      case T_CALLABLE:
      case T_UNRESOLVED_TYPE:
      case T_UNRESOLVED_NEWTYPE:
        nextLookahead(pos);
        break;
      case T_SHAPE:
        return tryParseShapeType(pos);
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
    if (pos->t != T_NS_SEPARATOR && pos->t != T_DOUBLE_COLON) {
      return true;
    }
    nextLookahead(pos);
  }
}

bool Scanner::tryParseShapeType(TokenStore::iterator& pos) {
  assert(pos->t == T_SHAPE);
  nextLookahead(pos);

  if (pos->t == T_STRING) {
    nextLookahead(pos);
    return true;
  }

  if (pos->t == '(') {
    nextLookahead(pos);
    if (pos->t != ')') {
      if (!tryParseShapeMemberList(pos)) return false;
      if (pos->t != ')') return false;
    }
    nextLookahead(pos);
    return true;
  }

  return false;
}

bool Scanner::tryParseShapeMemberList(TokenStore::iterator& pos) {
  assert(pos->t != ')'); // already determined to be nonempty

  for (;;) {
    if (!nextIfToken(pos, T_CONSTANT_ENCAPSED_STRING) ||
        !nextIfToken(pos, T_DOUBLE_ARROW)) {
      return false;
    }
    if (!tryParseNSType(pos)) return false;
    if (pos->t == ')') return true;
    if (!nextIfToken(pos, ',')) return false;
    if (pos->t == ')') return true;
  }

  return false;
}

static bool isUnresolved(int tokid) {
  return tokid == T_UNRESOLVED_LT ||
         tokid == T_UNRESOLVED_NEWTYPE ||
         tokid == T_UNRESOLVED_TYPE ||
         tokid == T_UNRESOLVED_OP;
}

int Scanner::getNextToken(ScannerToken &t, Location &l) {
  int tokid;
  bool la = !m_lookahead.empty();
  tokid = fetchToken(t, l);
  if (LIKELY(!isUnresolved(tokid))) {
    // In the common case, we don't have to perform any resolution
    // and we can just return the token
    if (UNLIKELY(la)) {
      // If we pulled a lookahead token, we need to remove it from
      // the lookahead store
      m_lookahead.popFront();
    }
    return tokid;
  }

  if (!la) {
    // If this token didn't come from the lookahead store, we
    // need to stash it there
    TokenStore::iterator it = m_lookahead.appendNew();
    LookaheadToken ltd = { t, l, tokid };
    *it = ltd;
  }

  switch (tokid) {
  case T_UNRESOLVED_NEWTYPE:
  case T_UNRESOLVED_TYPE: {
    auto pos = m_lookahead.begin();
    auto typePos = pos;
    nextLookahead(pos);
    if (pos->t == T_STRING) {
      typePos->t = tokid == T_UNRESOLVED_TYPE ? T_TYPE : T_NEWTYPE;
    } else {
      typePos->t = T_STRING;
    }
    break;
  }
  case T_UNRESOLVED_LT: {
    // Look at subsequent tokens to determine if the '<' character
    // is the start of a type list
    auto pos = m_lookahead.begin();
    auto ltPos = pos;
    nextLookahead(pos);
    ++m_lookaheadLtDepth;
    bool isTypeList = tryParseTypeList(pos);
    --m_lookaheadLtDepth;
    if (isTypeList && pos->t == '>') {
      ltPos->t = T_TYPELIST_LT;
      pos->t = T_TYPELIST_GT;
    } else {
      ltPos->t = '<';
    }
    break;
  }
  case T_UNRESOLVED_OP: {
    // Look at subsequent tokens to determine if the '(' character
    // is the start of a lambda expression
    auto pos = m_lookahead.begin();
    auto opPos = pos;
    nextLookahead(pos);
    if (pos->t != ')' && pos->t != T_LAMBDA_CP) {
      if (!tryParseNonEmptyLambdaParams(pos) || pos->t != ')') {
        opPos->t = '(';
        break;
      }
    }
    auto cpPos = pos;
    nextLookahead(pos);
    if (pos->t == ':') {
      nextLookahead(pos);
      if (!tryParseNSType(pos)) {
        opPos->t = '(';
        break;
      }
    }
    if (pos->t == T_LAMBDA_ARROW) {
      opPos->t = T_LAMBDA_OP;
      cpPos->t = T_LAMBDA_CP;
    } else {
      opPos->t = '(';
    }
    break;
  }
  default: always_assert(0);
  }

  tokid = fetchToken(t, l);
  // We pulled a lookahead token, we need to remove it from the
  // lookahead store
  m_lookahead.popFront();
  return tokid;
}

int Scanner::read(char *text, yy_size_t &result, yy_size_t max) {
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

int Scanner::read(char *text, int &result, yy_size_t max) {
  yy_size_t tmp;
  auto const ret = read(text, tmp, max);
  result = tmp;
  return ret;
}


void Scanner::error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(m_error, fmt, ap);
  va_end(ap);
}

void Scanner::warn(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::string msg;
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  Logger::Warning("%s: %s (Line: %d, Char %d)", msg.c_str(),
                  m_filename.c_str(), m_loc->r.line0, m_loc->r.char0);
}

void Scanner::incLoc(const char *rawText, int rawLeng, int type) {
  assert(rawText);
  assert(rawLeng > 0);
  if (m_listener) {
    m_token->setID(m_listener->publish(rawText, rawLeng, type));
  }

  m_loc->cursor += rawLeng;

  switch (m_state) {
    case Start:
      break; // scanner set to (1, 1, 1, 1) already
    case NoLineFeed:
      m_loc->r.line0 = m_loc->r.line1;
      m_loc->r.char0 = m_loc->r.char1 + 1;
      break;
    case HadLineFeed:
      m_loc->r.line0 = m_loc->r.line1 + 1;
      m_loc->r.char0 = 1;
      break;
  }
  const char *p = rawText;
  for (int i = 0; i < rawLeng; i++) {
    switch (m_state) {
      case Start:
        break; // scanner set to (1, 1, 1, 1) already
      case NoLineFeed:
        m_loc->r.char1++;
        break;
      case HadLineFeed:
        m_loc->r.line1++;
        m_loc->r.char1 = 1;
        break;
    }
    m_state = (*p++ == '\n' ? HadLineFeed : NoLineFeed);
  }
}

std::string Scanner::escape(const char *str, int len, char quote_type) const {
  std::string output;
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
          assert(false);
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
            case 'e':  output += '\033'; break;
            case '\\': output += '\\'; break;
            case '$':  output += '$';  break;
            case '"':
            case '`':
              if (str[i] != quote_type) {
                output += '\\';
              }
              output += str[i];
              break;
            case 'x':
            case 'X': {
              if (isxdigit(str[i+1])) {
                std::string shex;
                shex += str[++i]; // 0th hex digit
                if (isxdigit(str[i+1])) {
                  shex += str[++i]; // 1st hex digit
                }
                output += strtol(shex.c_str(), nullptr, 16);
              } else {
                output += ch;
                output += str[i];
              }
              break;
            }
            case 'u': {
              // Unicode escape sequence
              //   "\u{123456}"
              if (str[i+1] != '{') {
                // BC for "\u1234" passthrough
                output += ch;
                output += str[i];
                break;
              }

              bool valid = true;
              auto start = str + i + 2;
              auto closebrace = strchr(start, '}');
              if (closebrace > start) {
                for (auto p = start; p < closebrace; ++p) {
                  if (!isxdigit(*p)) {
                    valid = false;
                    break;
                  }
                }
              } else {
                valid = false;
              }

              auto fatal = [this](const char *msg) {
                auto loc = getLocation();
                return ParseTimeFatalException(
                  loc->file,
                  loc->r.line0,
                  "%s", msg);
              };
              if (!valid) {
                throw fatal("Invalid UTF-8 codepoint escape sequence");
              }

              std::string codepoint(start, closebrace - start);
              char *end = nullptr;
              int32_t uchar = strtol(codepoint.c_str(), &end, 16);
              if ((end && *end) || (uchar > 0x10FFFF)) {
                throw fatal(
                  "Invalid UTF-8 codepoint escape sequence: "
                  "Codepoint too large");
              }
              if (uchar <= 0x0007F) {
                output += (char)uchar;
              } else if (uchar <= 0x007FF) {
                output += (char)(0xC0 | ( uchar >> 6         ));
                output += (char)(0x80 | ( uchar        & 0x3F));
              } else if (uchar <= 0x00FFFF) {
                output += (char)(0xE0 | ( uchar >> 12        ));
                output += (char)(0x80 | ((uchar >>  6) & 0x3F));
                output += (char)(0x80 | ( uchar        & 0x3F));
              } else if (uchar <= 0x10FFFF) {
                output += (char)(0xF0 | ( uchar >> 18        ));
                output += (char)(0x80 | ((uchar >> 12) & 0x3F));
                output += (char)(0x80 | ((uchar >>  6) & 0x3F));
                output += (char)(0x80 | ( uchar        & 0x3F));
              } else {
                not_reached();
                assert(false);
              }
              i += codepoint.size() + 2 /* strlen("{}") */;
              break;
            }
            default: {
              // check for an octal
              if ('0' <= str[i] && str[i] <= '7') {
                std::string soct;
                soct += str[i]; // 0th octal digit
                if ('0' <= str[i+1] && str[i+1] <= '7') {
                  soct += str[++i];   // 1st octal digit
                  if ('0' <= str[i+1] && str[i+1] <= '7') {
                    soct += str[++i]; // 2nd octal digit
                  }
                }
                output += strtol(soct.c_str(), nullptr, 8);
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
  it.m_slab = nullptr;
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
  newSlab->m_next = nullptr;
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

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

#ifndef __HPHP_UTIL_PARSER_SCANNER_H__
#define __HPHP_UTIL_PARSER_SCANNER_H__

#include <sstream>
#include <util/exception.h>
#include <util/parser/location.h>
#include <util/parser/hphp.tab.hpp>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ScannerToken {
public:
  ScannerToken() : m_num(0), m_check(false) {}
  void reset() { m_num = 0; m_text.clear();}

  int num() const { return m_num;}
  void setNum(int num) {
    m_num = num;
  }
  void set(int num, const char *t) {
    m_num = num;
    m_text = t;
  }
  void set(int num, const std::string &t) {
    m_num = num;
    m_text = t;
  }
  void operator++(int) {
    ++m_num;
  }
  void operator=(ScannerToken &other) {
    m_num = other.m_num;
    m_text = other.m_text;
  }

  const std::string &text() const {
    return m_text;
  }
  bool same(const char *s) const {
    return strcasecmp(m_text.c_str(), s) == 0;
  }
  void setText(const char *t, int len) {
    m_text = std::string(t, len);
  }
  void setText(const char *t) {
    m_text = t;
  }
  void setText(const std::string &t) {
    m_text = t;
  }
  void setText(const ScannerToken &token) {
    m_text = token.m_text;
  }
  bool check() const {
    return m_check;
  }
  void setCheck() {
    m_check = true;
  }

  void xhpLabel(bool prefix = true);
  bool htmlTrim(); // true if non-empty after trimming
  void xhpDecode();  // xhp supports more entities than html

protected:
  int m_num; // internal token id
  std::string m_text;
  bool m_check;
};

///////////////////////////////////////////////////////////////////////////////

class Scanner {
public:
  enum Type {
    AllowShortTags   = 1, // allow <?
    AllowAspTags     = 2, // allow <% %>
    PreprocessXHP    = 4, // enable XHP
    ReturnAllTokens  = 8, // return comments and whitespaces
  };

public:
  Scanner(const char *filename, int type);
  Scanner(std::istream &stream, int type, const char *fileName = "");
  Scanner(const char *source, int len, int type, const char *fileName = "");
  ~Scanner();

  /**
   * Called by parser or tokenizer.
   */
  int getNextToken(ScannerToken &t, Location &l);
  const std::string &getError() const { return m_error;}
  Location *getLocation() const { return m_loc;}

  /**
   * Implemented in hphp.x, as they need to call yy functions.
   */
  void init();
  void reset();
  int scan();

  /**
   * Setting scanner states by parser.
   */
  void xhpCloseTag();
  void xhpChild();
  void xhpAttribute();
  void xhpStatement();
  void xhpAttributeDecl();
  void xhpReset();

  void setXhpState(int state) { m_xhpState = state;}
  int getXhpState() const { return m_xhpState;}
  bool isXhpState() const { return m_xhpState != 0;}

  bool hasGap() const { return m_gap;}
  bool inScript() const { return m_inScript;}
  void setInScript(bool inScript) { m_inScript = inScript;}

  /**
   * Called by lex.yy.cpp for YY_INPUT (see hphp.x)
   */
  int read(char *text, int &result, int max);

  /**
   * Called by scanner rules.
   */
  bool shortTags() const { return m_type & AllowShortTags;}
  bool aspTags() const { return m_type & AllowAspTags;}
  bool full() const { return m_type & ReturnAllTokens;}
  int lastToken() const { return m_lastToken;}
  void setToken(const char *rawText, int rawLeng) {
    m_token->setText(rawText, rawLeng);
    incLoc(rawText, rawLeng);
  }
  void stepPos(const char *rawText, int rawLeng) {
    if (m_type & ReturnAllTokens) {
      m_token->setText(rawText, rawLeng);
    }
    incLoc(rawText, rawLeng);
  }
  void setToken(const char *rawText, int rawLeng,
                const char *ytext, int yleng) {
    if (m_type & ReturnAllTokens) {
      m_token->setText(rawText, rawLeng);
    } else {
      m_token->setText(ytext, yleng);
    }
    incLoc(rawText, rawLeng);
  }
  void setHashBang(const char *rawText, int rawLeng);
  void error(const char* fmt, ...); // also used for YY_FATAL_ERROR in hphp.x
  void warn(const char* fmt, ...);
  std::string escape(char *str, int len, char quote_type) const;

  /**
   * Called by scanner rules for doc comments.
   */
  void setDocComment(const char *ytext, int yleng) {
    m_docComment.assign(ytext, yleng);
  }
  std::string detachDocComment() {
    std::string dc = m_docComment;
    m_docComment.clear();
    return dc;
  }

  /**
   * Called by scanner rules for HEREDOC/NOWDOC.
   */
  void setHeredocLabel(const char *label, int len) {
    m_heredocLabel.assign(label, len);
  }
  int getHeredocLabelLen() const {
    return m_heredocLabel.length();
  }
  const char *getHeredocLabel() const {
    return m_heredocLabel.data();
  }
  void resetHeredoc() {
    m_heredocLabel.clear();
  }

private:
  std::string m_filename;
  bool m_streamOwner;
  std::istream *m_stream;
  std::stringstream m_sstream; // XHP helper
  const char *m_source;
  int m_len;
  int m_pos;

  enum State {
    Start = -1,
    NoLineFeed,
    HadLineFeed,
  };
  State m_state;

  int m_type;
  void *m_yyscanner;
  ScannerToken *m_token;
  Location *m_loc;
  std::string m_error;

  std::string m_docComment;
  std::string m_heredocLabel;

  // fields for XHP parsing
  int m_lastToken;
  bool m_gap;      // was whitespace token
  bool m_inScript; // inside <script language="php"> </script>
  int m_xhpState;

  void incLoc(const char *rawText, int rawLeng);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_UTIL_PARSER_SCANNER_H__

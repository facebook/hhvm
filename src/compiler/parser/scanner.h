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

#ifndef __HPHP_SCANNER_H__
#define __HPHP_SCANNER_H__

#include <util/ylmm/basic_scanner.hh>
#include <sstream>
#include <compiler/hphp.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(Statement);

class Token {
public:
  Token() : num(0) {}

  void setTexts(const char *rawtext, int rawleng,
                const char *yytext, int yyleng);
  void setText(const char *text);

  int num;             // internal token id
  ExpressionPtr exp;
  StatementPtr stmt;

  const std::string &text() { return tokenText.get() ? *tokenText : s_empty;}
  const std::string &rawtext() { return rawText.get() ? *rawText : s_empty;}

  Token &operator=(int num) { this->num = num; return *this;}
  void operator++(int) { this->num++;}

  void reset() {
    tokenText.reset(); rawText.reset(); num = 0; exp.reset(); stmt.reset();
  }

private:
  const static std::string s_empty;

  // original token text from the source script
  boost::shared_ptr<std::string> rawText;
  // token text after lexical analysis
  boost::shared_ptr<std::string> tokenText;
};

inline std::ostream &operator<< (std::ostream &o, const Token &e) {
  return o;
}

class Scanner : public ylmm::basic_scanner<Token> {
public:
  Scanner(ylmm::basic_buffer* buf, bool bShortTags, bool bASPTags);
  ~Scanner() { switch_buffer(0);}
  void setToken(const char *rawText, int rawLeng,
                const char *yytext, int yyleng);

  bool shortTags() const { return m_shortTags;}
  bool aspTags() const { return m_aspTags;}
  void setHeredocLabel(const char *label, int len);
  int getHeredocLabelLen() const;
  const char *getHeredocLabel() const;
  void resetHeredoc();
  virtual int wrap() { return 1;}
  int getNextToken(token_type& t, location_type& l);
  std::string scanEscapeString(char *str, int len, char quote_type) const;

  std::string getError() const { return m_err.str();}
  std::string getMessage() const { return m_msg.str();}
  int getLine() const { return m_line;}
  int getColumn() const { return m_column;}
  void setDocComment(const char *yytext, int yyleng);
  std::string getDocComment() {
    std::string dc = m_docComment;
    m_docComment = "";
    return dc;
  }

protected:
  std::ostringstream m_err;
  std::ostringstream m_msg;
  ylmm::basic_messenger<ylmm::basic_lock> m_messenger;
  bool m_shortTags;
  bool m_aspTags;
  std::string m_heredocLabel;
  int m_line;   // last token line
  int m_column; // last token column
  std::string m_docComment;
};

///////////////////////////////////////////////////////////////////////////////
}

extern void _scanner_init();

#endif // __HPHP_SCANNER_H__

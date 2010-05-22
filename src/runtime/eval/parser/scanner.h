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

#ifndef __EVAL_SCANNER_H__
#define __EVAL_SCANNER_H__

#include <runtime/eval/parser/parser_defines.h>
#include <util/ylmm/basic_scanner.hh>
#include <sstream>
#include <runtime/eval/base/eval_base.h>
#include <runtime/eval/ast/statement.h>
#include <runtime/eval/ast/expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(TokenPayload);
DECLARE_AST_PTR(IfBranch);
DECLARE_AST_PTR(StatementListStatement);
DECLARE_AST_PTR(CaseStatement);
DECLARE_AST_PTR(ListElement);
DECLARE_AST_PTR(ArrayPair);
DECLARE_AST_PTR(CatchBlock);
DECLARE_AST_PTR(Parameter);
DECLARE_AST_PTR(Name);
DECLARE_AST_PTR(StaticVariable);

class TokenPayload {
public:
  enum Mode {
    None,
    SingleExpression,
    SingleStatement,
    SingleName,
    IfBranch,
    CaseStatement,
    Expression,
    ListElement,
    ArrayPair,
    CatchBlock,
    Parameter,
    Name,
    StaticVariable,
    Strings
  };

  TokenPayload();
  ~TokenPayload();
  void release();
  template<class T>
  AstPtr<T> getExp() const {
    return m_exp ? m_exp->cast<T>() : AstPtr<T>();
  }
  template<class T>
  AstPtr<T> getStmt() const {
    return m_stmt ? m_stmt->cast<T>() : AstPtr<T>();
  }
  StatementListStatementPtr getStmtList() const;

  ExpressionPtr &exp();
  StatementPtr &stmt();
  NamePtr &name();
#define GETTER(type, data) std::vector<type##Ptr> &data();
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

  std::vector<String> &strings();

  Mode getMode() const { return m_mode; }

private:
  ExpressionPtr m_exp;
  StatementPtr m_stmt;
  NamePtr m_name;
  Mode m_mode;
  union {
    std::vector<IfBranchPtr> *ifBranches;
    std::vector<CaseStatementPtr> *cases;
    std::vector<ExpressionPtr> *exprs;
    std::vector<ListElementPtr> *listElems;
    std::vector<ArrayPairPtr> *arrayPairs;
    std::vector<CatchBlockPtr> *catches;
    std::vector<ParameterPtr> *params;
    std::vector<NamePtr> *names;
    std::vector<StaticVariablePtr> *staticVars;
    std::vector<String> *strings;
  } m_data;
};

class Token {
public:
   Token() : num(0) {}
  ~Token() { reset(); }
  // token text after lexical analysis
  boost::shared_ptr<std::string> text;
  int num;             // internal token id


  const std::string &getText() const {
    if (text.get()) return *text.get();
    else return s_empty;
  }
  void setText(const char *t);
  TokenPayloadPtr &operator->();
  Token &operator=(Token &other);
  Token &operator=(int num) { this->num = num; return *this;}
  void operator++(int) { this->num++;}
  void reset();

private:
  TokenPayloadPtr m_payload;
  const static std::string s_empty;
};

inline std::ostream &operator<< (std::ostream &o, const Token &e) {
  return o;
}

class Scanner : public ylmm::basic_scanner<Token> {
public:
  Scanner(ylmm::basic_buffer* buf, bool bShortTags, bool bASPTags,
          bool full = false);
  ~Scanner() { switch_buffer(0);}
  void setToken(const char *rawText, int rawLeng,
                const char *yytext, int yyleng, bool save = true);
  bool shortTags() const { return m_shortTags;}
  bool aspTags() const { return m_aspTags;}
  void setHeredocLabel(const char *label, int len);
  int getHeredocLabelLen() const;
  const char *getHeredocLabel() const;
  void resetHeredoc();
  int wrap() { return 1;}
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
  void flushFlex();
protected:
  std::ostringstream m_err;
  std::ostringstream m_msg;
  ylmm::basic_messenger<ylmm::basic_lock> m_messenger;
  bool m_shortTags;
  bool m_aspTags;
  bool m_full;
  std::string m_heredocLabel;
  int m_line;   // last token line
  int m_column; // last token column
  std::string m_docComment;
  void incLoc(const char *yytext, int yyleng);
};
}
}

extern void _eval_scanner_init();
///////////////////////////////////////////////////////////////////////////////

#endif // __EVAL_SCANNER_H__

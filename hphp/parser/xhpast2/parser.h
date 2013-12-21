/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_PARSER_XHPAST2_H_
#define incl_HPHP_PARSER_XHPAST2_H_

#include <stdexcept>
#include <iostream>
#include <cstdarg>
#include "folly/Format.h"
#include "folly/String.h"
#include "hphp/parser/parser.h"
#include "astnode.hpp"

#define HPHP_PARSER_NS XHPAST2

#define HPHP_PARSER_ERROR(fmt, p, args...)      \
  throw std::runtime_error(folly::format(       \
    "{}:{}:{}", (p)->file(), (p)->line1(),      \
      folly::stringPrintf(fmt, ##args)).str())

namespace HPHP { namespace HPHP_PARSER_NS {

extern bool g_verifyMode;

//////////////////////////////////////////////////////////////////////

struct ExtraInfo {
  virtual ~ExtraInfo() {}
};

struct OnNameEI : ExtraInfo {
  HPHP::ParserBase::NameKind kind;
  explicit OnNameEI(HPHP::ParserBase::NameKind k) : kind(k) {};
};

struct OnVariableEI : ExtraInfo {
  bool constant;
  const std::string& docComment;
  OnVariableEI(bool c, const std::string& dc) : constant(c), docComment(dc) {}
};

struct OnDynamicVariableEI : ExtraInfo {
  bool encap;
  explicit OnDynamicVariableEI(bool e) : encap(e) {}
};

struct OnCallParamEI : ExtraInfo {
  bool ref;
  explicit OnCallParamEI(bool r) : ref(r) {}
};

struct OnCallEI : ExtraInfo {
  bool dynamic;
  bool fromCompiler;
  OnCallEI(bool d, bool f) : dynamic(d), fromCompiler(f) {}
};

struct OnEncapsListEI : ExtraInfo {
  int type;
  explicit OnEncapsListEI(int t) : type(t) {}
};

struct AddEncapEI : ExtraInfo {
  int type;
  explicit AddEncapEI(int t) : type(t) {}
};

struct OnScalarEI : ExtraInfo {
  int type;
  explicit OnScalarEI(int t) : type(t) {}
};

struct OnListAssignmentEI : ExtraInfo {
  bool rhsFirst;
  explicit OnListAssignmentEI(bool rhs) : rhsFirst(rhs) {}
};

struct OnAssignEI : ExtraInfo {
  bool ref;
  bool rhsFirst;
  OnAssignEI(bool r, bool rhs) : ref(r), rhsFirst(rhs) {}
};

struct OnUnaryOpExpEI : ExtraInfo {
  int op;
  bool front;
  OnUnaryOpExpEI(int o, bool f) : op(o), front(f) {}
};

struct OnBinaryOpExpEI : ExtraInfo {
  int op;
  explicit OnBinaryOpExpEI(int o) : op(o) {}
};

struct OnArrayEI : ExtraInfo {
  int op;
  explicit OnArrayEI(int o) : op(o) {}
};

struct OnArrayPairEI : ExtraInfo {
  bool ref;
  explicit OnArrayPairEI(bool r) : ref(r) {}
};

struct OnClassConstEI : ExtraInfo {
  bool text;
  explicit OnClassConstEI(bool t) : text(t) {}
};

struct OnParamEI : ExtraInfo {
  bool ref;
  explicit OnParamEI(bool r) : ref(r) {}
};

struct OnClassEI : ExtraInfo {
  int type;
  explicit OnClassEI(int t) : type(t) {}
};

struct OnMethodEI : ExtraInfo {
  bool reloc;
  explicit OnMethodEI(bool r) : reloc(r) {}
};

struct OnReturnEI : ExtraInfo {
  bool checkYield;
  explicit OnReturnEI(bool c) : checkYield(c) {}
};

struct OnEchoEI : ExtraInfo {
  bool html;
  explicit OnEchoEI(bool h) : html(h) {}
};

struct OnClosureEI : ExtraInfo {
  bool is_static;
  explicit OnClosureEI(bool i) : is_static(i) {}
};

struct OnClosureParamEI : ExtraInfo {
  bool ref;
  explicit OnClosureParamEI(bool r) : ref(r) {}
};

struct OnGotoEI : ExtraInfo {
  bool limited;
  explicit OnGotoEI(bool l) : limited(l) {}
};

struct OnTypeSpecializationEI : ExtraInfo {
  char specialization;
  explicit OnTypeSpecializationEI(char s) : specialization(s) {}
};

enum NodeType {
  RAW = 0,
  ONNAME = 1,
  ONVARIABLE = 2,
  ONSTATICVARIABLE = 3,
  ONCLASSVARIABLESTART = 4,
  ONCLASSVARIABLE = 5,
  ONCLASSCONSTANT = 6,
  ONSIMPLEVARIABLE = 7,
  ONSYNTHESIZEDVARIABLE = 8,
  ONDYNAMICVARIABLE = 9,
  ONINDIRECTREF = 10,
  ONSTATICMEMBER = 11,
  ONREFDIM = 12,
  ONCALLPARAM = 13,
  ONCALL = 14,
  ONENCAPSLIST = 15,
  ADDENCAP = 16,
  ENCAPREFDIM = 17,
  ENCAPOBJPROP = 18,
  ENCAPARRAY = 19,
  ONCONSTANTVALUE = 20,
  ONSCALAR = 21,
  ONEXPRLISTELEM = 22,
  ONOBJECTPROPERTY = 23,
  ONOBJECTMETHODCALL = 24,
  ONLISTASSIGNMENT = 25,
  ONALISTVAR = 26,
  ONALISTSUB = 27,
  ONASSIGN = 28,
  ONASSIGNNEW = 29,
  ONNEWOBJECT = 30,
  ONUNARYOPEXP = 31,
  ONBINARYOPEXP = 32,
  ONQOP = 33,
  ONARRAY = 34,
  ONARRAYPAIR = 35,
  ONCOLLECTIONPAIR = 36,
  ONUSERATTRIBUTE = 37,
  ONCLASSCONST = 38,
  ONFUNCTION = 39,
  ONPARAM = 40,
  ONCLASS = 41,
  ONINTERFACE = 42,
  ONINTERFACENAME = 43,
  ONTRAITUSE = 44,
  ONTRAITNAME = 45,
  ONTRAITRULE = 46,
  ONTRAITPRECRULE = 47,
  ONTRAITALIASRULESTART = 48,
  ONTRAITALIASRULEMODIFY = 49,
  ONMETHOD = 50,
  ONMEMBERMODIFIER = 51,
  ONSTATEMENTLISTSTART = 52,
  ADDSTATEMENT = 53,
  ONCLASSSTATEMENT = 54,
  FINISHSTATEMENT = 55,
  ONBLOCK = 56,
  ONIF = 57,
  ONELSEIF = 58,
  ONWHILE = 59,
  ONDO = 60,
  ONFOR = 61,
  ONSWITCH = 62,
  ONCASE = 63,
  ONBREAK = 64,
  ONCONTINUE = 65,
  ONRETURN = 66,
  ONYIELD = 67,
  ONYIELDBREAK = 68,
  ONGLOBAL = 69,
  ONGLOBALVAR = 70,
  ONSTATIC = 71,
  ONECHO = 72,
  ONUNSET = 73,
  ONEXPSTATEMENT = 74,
  ONFOREACH = 75,
  ONTRYCATCHFINALLY = 76,
  ONTRYFINALLY = 77,
  ONCATCH = 78,
  ONFINALLY = 79,
  ONTHROW = 80,
  ONCLOSURE = 81,
  ONCLOSUREPARAM = 82,
  ONLABEL = 83,
  ONGOTO = 84,
  ONTYPEDEF = 85,
  ONTYPEANNOTATION = 86,
  ONTYPELIST = 87,
  ONTYPESPECIALIZATION = 88,
  ONYIELDPAIR = 89,
  ONAWAIT = 90,
  ONQUERY = 91,
  ONQUERYBODY = 92,
  ONQUERYBODYCLAUSE = 93,
  ONFROMCLAUSE = 94,
  ONLETCLAUSE = 95,
  ONWHERECLAUSE = 96,
  ONJOINCLAUSE = 97,
  ONJOININTOCLAUSE = 98,
  ONORDERBYCLAUSE = 99,
  ONORDERING = 100,
  ONORDERINGEXPR = 101,
  ONSELECTCLAUSE = 102,
  ONGROUPCLAUSE = 103,
  ONINTOCLAUSE = 104,
  ONTRAITREQUIREEXTEND = 105,
  ONTRAITREQUIREIMPLEMENT = 106,
};

struct Token : ScannerToken {
  // this parser implementation could leak memory and is unsuitable for long
  // running processes
  NodeType nodeType;
  ExtraInfo *extra;
  std::vector<Token *> children;
  Token() : nodeType(RAW),extra(nullptr) {}
  Token& operator=(int num) {
    ScannerToken::m_num = num;
    return *this;
  }
  Token& operator=(const Token& other) {
    ScannerToken::operator=(other);
    nodeType = other.nodeType;
    extra = other.extra;
    children.clear();
    appendChildren(&other);
    return *this;
  }
  Token* operator->() {
    return this;
  }
  Token& operator+(const char* str) {
    ScannerToken::m_text += str;
    return *this;
  }
  Token& operator+(const Token& token) {
    ScannerToken::m_num += token.m_num;
    ScannerToken::m_text += token.m_text;
    return *this;
  }
  Token& appendChild(Token *token) {
    if (token) {
      Token *c = new Token();
      *c = *token;
      children.push_back(c);
    } else {
      children.push_back(token);
    }
    return *this;
  }
  Token& appendChildren(const Token* token) {
    for (auto& c : token->children) {
      children.push_back(c);
    }
    return *this;
  }
  Token& setNodeType(NodeType t) {
    nodeType = t;
    children.clear();
    extra = nullptr;
    return *this;
  }
  Token& setExtra(ExtraInfo *ei) {
    extra = ei;
    return *this;
  }
  friend std::ostream& operator<<(std::ostream& out, Token& t) {
    if (t.nodeType == RAW) {
      out << "RAW:" << t.m_text;
    } else {
      out << "[" << t.nodeType << ":" << t.m_num << ":" << t.m_id;
      if (t.children.size() > 0) {
        std::cout << " [";
        for (int i = 0; i < t.children.size(); i++) {
          Token *c = t.children[i];
          if (c) {
            out << *c;
          } else {
            out << "NULL";
          }
          if (i < t.children.size() - 1) {
            out << ",";
          }
        }
        std::cout << "]";
      }
      out << "]";
    }
    return out;
//    return out << t.m_text << ":" << t.m_num << ":" << t.m_id;
  }
};

struct XHPASTTokenListener : HPHP::TokenListener {
  std::vector<xhpast::Token*> tokens;
  virtual int publish(const char *rawText, int rawLeng, int type) {
    // single chars like ':' with no explicit token type
    if ((type == -1) && (rawLeng == 1)) {
      type = (unsigned int)rawText[0];
    }
    tokens.push_back(
      new xhpast::Token((unsigned int)type,
                        const_cast<char*>(rawText),
                        (unsigned int)rawLeng)
    );
    return tokens.size() - 1;
  }
  friend std::ostream& operator<<(std::ostream& out, XHPASTTokenListener& l) {
    for (std::vector<xhpast::Token*>::iterator it = l.tokens.begin();
         it != l.tokens.end(); ++it) {
      out << "[" << (*it)->type << "," << (*it)->value.size() << "]";
    }
    return out;
  }
  ~XHPASTTokenListener() {
    for (std::vector<xhpast::Token*>::iterator it = tokens.begin();
         it != tokens.end(); ++it) {
//      std::cout << "[" << (*it)->value << "," << (*it)->type << "]";
      delete (*it);
    }
  }
};

/*
 * Parser that creates an AST
 */
struct Parser : ParserBase {
  XHPASTTokenListener m_listener;

  explicit Parser(Scanner& scanner,
                  const char* filename)
      : ParserBase(scanner, filename) {
    scanner.setListener(dynamic_cast<HPHP::TokenListener*>(&m_listener));
  }

#define X(...)
//#define X(...) traceCb(__FUNCTION__,## __VA_ARGS__)

  void fatal(const Location* loc, const char* msg) {
    throw std::runtime_error(folly::format(
      "{}:{}: {}", m_fileName, m_loc.line0, msg).str());
  }

  void parseFatal(const Location* loc, const char* msg) {
    throw std::runtime_error(folly::format(
      "{}:{}: {}", m_fileName, m_loc.line0, msg).str());
  }

  void parse() {

    if (!parseImpl()) {
      error("parse failure: %s\n", getMessage().c_str());
    }
  }

 public:
  bool parseImpl();

  void error(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    std::string msg;
    Util::string_vsnprintf(msg, fmt, ap);
    va_end(ap);

    throw std::runtime_error(folly::format(
      "{}:{}: {}", m_fileName, m_loc.line0, msg).str());
  }

  void invalidateGoto(TStatementPtr, ParserBase::GotoError) { }
  void invalidateLabel(TStatementPtr) { }
  void* extractStatement(ScannerToken*) { return nullptr; }

  bool enableFinallyStatement() { return true; }

  IMPLEMENT_XHP_ATTRIBUTES;

  Token tree;

  void initParseTree() {}
  void finiParseTree() {}

  void onName(Token& out, Token& name, NameKind kind) {
    out.setNodeType(ONNAME).setExtra(new OnNameEI(kind)).appendChild(&name);
  }

  void onVariable(Token& out, Token* exprs, Token& var, Token* value,
                  bool constant = false,
                  const std::string& docComment = "") {
    out.setNodeType(ONVARIABLE).setExtra(new OnVariableEI(constant, docComment))
      .appendChild(exprs).appendChild(&var).appendChild(value);
  }

  void onStaticVariable(Token &out, Token *exprs, Token &var, Token *value) {
    out.setNodeType(ONSTATICVARIABLE).appendChild(exprs).appendChild(&var)
      .appendChild(value);
  }

  void onClassVariableModifer(Token &mod) { /* TODO */
    X(mod);
  }

  void onClassVariableStart(Token &out, Token *modifiers, Token &decl,
                            Token *type) {
    out.setNodeType(ONCLASSVARIABLESTART).appendChild(modifiers)
      .appendChild(&decl).appendChild(type);
  }

  void onClassVariable(Token &out, Token *exprs, Token &var, Token *value) {
    out.setNodeType(ONCLASSVARIABLE).appendChild(exprs).appendChild(&var)
      .appendChild(value);
  }

  void onClassConstant(Token &out, Token *exprs, Token &var, Token &value) {
    out.setNodeType(ONCLASSCONSTANT).appendChild(exprs).appendChild(&var)
      .appendChild(&value);
  }

  void onSimpleVariable(Token &out, Token &var) {
    out.setNodeType(ONSIMPLEVARIABLE).appendChild(&var);
  }

  void onSynthesizedVariable(Token& out, Token &var) {
    out.setNodeType(ONSYNTHESIZEDVARIABLE).appendChild(&var);
  }

  void onDynamicVariable(Token &out, Token &expr, bool encap) {
    out.setNodeType(ONDYNAMICVARIABLE).appendChild(&expr)
      .setExtra(new OnDynamicVariableEI(encap));
  }

  void onIndirectRef(Token &out, Token &refCount, Token &var) {
    out.setNodeType(ONINDIRECTREF).appendChild(&refCount).appendChild(&var);
  }

  void onStaticMember(Token &out, Token &cls, Token &name) {
    out.setNodeType(ONSTATICMEMBER).appendChild(&cls).appendChild(&name);
  }

  void onRefDim(Token &out, Token &var, Token &offset) {
    out.setNodeType(ONREFDIM).appendChild(&var).appendChild(&offset);
  }

  void onCallParam(Token &out, Token *params, Token &expr, bool ref) {
    out.setNodeType(ONCALLPARAM).appendChild(params).appendChild(&expr)
      .setExtra(new OnCallParamEI(ref));
  }

  void onCall(Token &out, bool dynamic, Token &name, Token &params,
              Token *cls, bool fromCompiler = false) {
    out.setNodeType(ONCALL).appendChild(&name).appendChild(&params)
      .appendChild(cls).setExtra(new OnCallEI(dynamic, fromCompiler));
  }

  void onEncapsList(Token &out, int type, Token &list) {
    out.setNodeType(ONENCAPSLIST).appendChild(&list)
      .setExtra(new OnEncapsListEI(type));
  }

  void addEncap(Token &out, Token *list, Token &expr, int type) {
    out.setNodeType(ADDENCAP).appendChild(list).appendChild(&expr)
      .setExtra(new AddEncapEI(type));
  }

  void encapRefDim(Token &out, Token &var, Token &offset) {
    out.setNodeType(ENCAPREFDIM).appendChild(&var).appendChild(&offset);
  }

  void encapObjProp(Token &out, Token &var, Token &name) {
    out.setNodeType(ENCAPOBJPROP).appendChild(&var).appendChild(&name);
  }

  void encapArray(Token &out, Token &var, Token &expr) {
    out.setNodeType(ENCAPARRAY).appendChild(&var).appendChild(&expr);
  }

  void onConstantValue(Token &out, Token &constant) {
    out.setNodeType(ONCONSTANTVALUE).appendChild(&constant);
  }

  void onScalar(Token &out, int type, Token &scalar) {
    out.setNodeType(ONSCALAR).appendChild(&scalar)
      .setExtra(new OnScalarEI(type));
  }

  void onExprListElem(Token &out, Token *exprs, Token &expr) {
    out.setNodeType(ONEXPRLISTELEM).appendChild(exprs).appendChild(&expr);
  }

  void onObjectProperty(Token &out, Token &base, Token &prop) {
    out.setNodeType(ONOBJECTPROPERTY).appendChild(&base).appendChild(&prop);
  }

  void onObjectMethodCall(Token &out, Token &base, Token &prop,
                          Token &params) {
    out.setNodeType(ONOBJECTMETHODCALL).appendChild(&base).appendChild(&prop)
      .appendChild(&params);
  }

  void onListAssignment(Token &out, Token &vars, Token *expr,
                        bool rhsFirst = false) {
    out.setNodeType(ONLISTASSIGNMENT).appendChild(&vars).appendChild(expr)
      .setExtra(new OnListAssignmentEI(rhsFirst));
  }

  void onAListVar(Token &out, Token *list, Token *var) {
    out.setNodeType(ONALISTVAR).appendChild(list).appendChild(var);
  }

  void onAListSub(Token &out, Token *list, Token &sublist) {
    out.setNodeType(ONALISTSUB).appendChild(list).appendChild(&sublist);
  }

  void onAssign(Token& out, Token& var, Token& expr, bool ref,
                bool rhsFirst = false) {
    out.setNodeType(ONASSIGN).appendChild(&var).appendChild(&expr)
      .setExtra(new OnAssignEI(ref, rhsFirst));
  }

  void onAssignNew(Token &out, Token &var, Token &name, Token &args) {
    out.setNodeType(ONASSIGNNEW).appendChild(&var).appendChild(&name)
      .appendChild(&args);
  }

  void onNewObject(Token &out, Token &name, Token &args) {
    out.setNodeType(ONNEWOBJECT).appendChild(&name).appendChild(&args);
  }

  void onUnaryOpExp(Token &out, Token &operand, int op, bool front) {
    out.setNodeType(ONUNARYOPEXP).appendChild(&operand)
      .setExtra(new OnUnaryOpExpEI(op, front));
  }

  void onBinaryOpExp(Token &out, Token &operand1, Token &operand2, int op) {
    out.setNodeType(ONBINARYOPEXP).appendChild(&operand1).appendChild(&operand2)
      .setExtra(new OnBinaryOpExpEI(op));
  }

  void onQOp(Token &out, Token &exprCond, Token *expYes, Token &expNo) {
    out.setNodeType(ONQOP).appendChild(&exprCond).appendChild(expYes)
      .appendChild(&expNo);
  }

  void onArray(Token &out, Token &pairs, int op = T_ARRAY) {
    out.setNodeType(ONARRAY).appendChild(&pairs)
      .setExtra(new OnArrayEI(op));
  }

  void onArrayPair(Token &out, Token *pairs, Token *name, Token &value,
                   bool ref) {
    out.setNodeType(ONARRAYPAIR).appendChild(pairs).appendChild(name)
      .appendChild(&value).setExtra(new OnArrayPairEI(ref));
  }

  void onEmptyCollection(Token &out) { X(); /* TODO */}

  void onCollectionPair(Token &out, Token *pairs, Token *name, Token &value) {
    out.setNodeType(ONCOLLECTIONPAIR).appendChild(pairs).appendChild(name)
      .appendChild(&value);
  }

  void onUserAttribute(Token &out, Token *attrList, Token &name,
                       Token &value) {
    out.setNodeType(ONUSERATTRIBUTE).appendChild(attrList).appendChild(&name)
      .appendChild(&value);
  }

  void onClassConst(Token &out, Token &cls, Token &name, bool text) {
    out.setNodeType(ONCLASSCONST).appendChild(&cls).appendChild(&name)
      .setExtra(new OnClassConstEI(text));
  }

  void fixStaticVars() { /* TODO */}

  void onFunctionStart(Token& name, bool doPushComment = true) {
    /* TODO */
  }

  void onClosureStart(Token& name) {
    /* TODO */
  }

  void onFunction(Token& out, Token *modifiers, Token& ret, Token& ref,
                  Token& name, Token& params, Token& stmt, Token* attr) {
    out.setNodeType(ONFUNCTION).appendChild(modifiers).appendChild(&ret)
      .appendChild(&ref).appendChild(&name).appendChild(&params)
      .appendChild(&stmt).appendChild(attr);
  }

  void onParam(Token &out, Token *params, Token &type, Token &var,
               bool ref, Token *defValue, Token *attr, Token *mods) {
    out.setNodeType(ONPARAM).appendChild(params).appendChild(&type)
      .appendChild(&var).appendChild(defValue).appendChild(attr)
      .appendChild(mods).setExtra(new OnParamEI(ref));
  }

  void onClassStart(int type, Token &name) {
    /* TODO */
  }

  void onClass(Token &out, int type, Token &name, Token &base,
               Token &baseInterface, Token &stmt, Token *attr) {
    out.setNodeType(ONCLASS).appendChild(&name).appendChild(&base)
      .appendChild(&baseInterface).appendChild(&stmt).appendChild(attr)
      .setExtra(new OnClassEI(type));
  }

  void onInterface(Token &out, Token &name, Token &base, Token &stmt,
                   Token *attr) {
    out.setNodeType(ONINTERFACE).appendChild(&name).appendChild(&base)
      .appendChild(&stmt).appendChild(attr);
  }

  void onInterfaceName(Token &out, Token *names, Token &name) {
    out.setNodeType(ONINTERFACENAME).appendChild(names).appendChild(&name);
  }

  void onTraitUse(Token &out, Token &traits, Token &rules) {
    out.setNodeType(ONTRAITUSE).appendChild(&traits).appendChild(&rules);
  }

  void onTraitRequire(Token &out, Token &name, bool isExtends) {
    out.setNodeType(isExtends ? ONTRAITREQUIREEXTEND : ONTRAITREQUIREIMPLEMENT)
      .appendChild(&name);
  }

  void onTraitName(Token &out, Token *names, Token &name) {
    out.setNodeType(ONTRAITNAME).appendChild(names).appendChild(&name);
  }

  void onTraitRule(Token &out, Token &stmtList, Token &newStmt) {
    out.setNodeType(ONTRAITRULE).appendChild(&stmtList).appendChild(&newStmt);
  }

  void onTraitPrecRule(Token &out, Token &className, Token &methodName,
                       Token &otherClasses) {
    out.setNodeType(ONTRAITPRECRULE).appendChild(&className)
      .appendChild(&methodName).appendChild(&otherClasses);
  }

  void onTraitAliasRuleStart(Token &out, Token &className, Token &methodName) {
    out.setNodeType(ONTRAITALIASRULESTART).appendChild(&className)
      .appendChild(&methodName);
  }

  void onTraitAliasRuleModify(Token &out, Token &rule, Token &accessModifiers,
                              Token &newMethodName) {
    out.setNodeType(ONTRAITALIASRULEMODIFY).appendChild(&rule)
      .appendChild(&accessModifiers).appendChild(&newMethodName);
  }

  void onMethodStart(Token &name, Token &mods, bool doPushComment = true) {
    /* TODO */
  }

  void onMethod(Token &out, Token &modifiers, Token &ret, Token &ref,
                Token &name, Token &params, Token &stmt, Token *attr,
                bool reloc = true) {
    // modifiers could be garbage Tokens if no modifier specified.
    out.setNodeType(ONMETHOD).appendChild(&modifiers).appendChild(&ret)
      .appendChild(&ref).appendChild(&name).appendChild(&params)
      .appendChild(&stmt).appendChild(attr).setExtra(new OnMethodEI(reloc));
  }

  void onMemberModifier(Token &out, Token *modifiers, Token &modifier) {
    out.setNodeType(ONMEMBERMODIFIER).appendChild(modifiers)
      .appendChild(&modifier);
  }

  void onStatementListStart(Token &out) {
    out.setNodeType(ONSTATEMENTLISTSTART);
  }

  void addTopStatement(Token &new_stmt) { this->tree = new_stmt; }

  // TODO
  void onHaltCompiler() {}

  void addStatement(Token& out, Token& stmts, Token& new_stmt) {
    out.setNodeType(ADDSTATEMENT).appendChild(&stmts).appendChild(&new_stmt);
  }

  void onClassStatement(Token &out, Token &stmts, Token &new_stmt) {
    out.setNodeType(ONCLASSSTATEMENT).appendChild(&stmts)
      .appendChild(&new_stmt);
  }

  void finishStatement(Token& out, Token& stmts) {
    out.setNodeType(FINISHSTATEMENT).appendChild(&stmts);
  }

  void onBlock(Token& out, Token& stmts) {
    out.setNodeType(ONBLOCK).appendChild(&stmts);
  }

  void onIf(Token &out, Token &cond, Token &stmt, Token &elseifs,
            Token &elseStmt) {
    out.setNodeType(ONIF).appendChild(&cond).appendChild(&stmt)
      .appendChild(&elseifs).appendChild(&elseStmt);
  }

  void onElseIf(Token& out, Token& elseifs, Token& cond, Token& stmt) {
    out.setNodeType(ONELSEIF).appendChild(&elseifs).appendChild(&cond)
      .appendChild(&stmt);
  }

  void onWhile(Token &out, Token &cond, Token &stmt) {
    out.setNodeType(ONWHILE).appendChild(&cond).appendChild(&stmt);
  }

  void onDo(Token &out, Token &stmt, Token &cond) {
    out.setNodeType(ONDO).appendChild(&stmt).appendChild(&cond);
  }

  void onFor(Token &out, Token &expr1, Token &expr2, Token &expr3,
             Token &stmt) {
    out.setNodeType(ONFOR).appendChild(&expr1).appendChild(&expr2)
      .appendChild(&expr3).appendChild(&stmt);
  }

  void onSwitch(Token &out, Token &expr, Token &cases) {
    out.setNodeType(ONSWITCH).appendChild(&expr).appendChild(&cases);
  }

  void onCase(Token &out, Token &cases, Token *cond, Token &stmt) {
    out.setNodeType(ONCASE).appendChild(&cases).appendChild(cond)
      .appendChild(&stmt);
  }

  void onBreakContinue(Token &out, bool isBreak, Token *expr) {
    if (isBreak) {
      out.setNodeType(ONBREAK).appendChild(expr);
    } else {
      out.setNodeType(ONCONTINUE).appendChild(expr);
    }
  }

  void onReturn(Token &out, Token *expr, bool checkYield = true) {
    out.setNodeType(ONRETURN).appendChild(expr)
      .setExtra(new OnReturnEI(checkYield));
  }

  void onYield(Token &out, Token &expr) {
    out.setNodeType(ONYIELD).appendChild(&expr);
  }

  void onYieldPair(Token &out, Token &key, Token &val) {
    out.setNodeType(ONYIELDPAIR).appendChild(&key).appendChild(&val);
  }

  void onYieldBreak(Token &out) {
    out.setNodeType(ONYIELDBREAK);
  }

  void onAwait(Token &out, Token &expr) {
    out.setNodeType(ONAWAIT).appendChild(&expr);
  }

  void onGlobal(Token &out, Token &expr) {
    out.setNodeType(ONGLOBAL).appendChild(&expr);
  }
  void onGlobalVar(Token &out, Token *exprs, Token &expr) {
    out.setNodeType(ONGLOBALVAR).appendChild(exprs).appendChild(&expr);
  }

  void onStatic(Token &out, Token &expr) {
    out.setNodeType(ONSTATIC).appendChild(&expr);
  }
  void onEcho(Token &out, Token &expr, bool html) {
    out.setNodeType(ONECHO).appendChild(&expr).setExtra(new OnEchoEI(html));
  }
  void onUnset(Token &out, Token &expr) {
    out.setNodeType(ONUNSET).appendChild(&expr);
  }

  void onExpStatement(Token& out, Token& expr) {
    out.setNodeType(ONEXPSTATEMENT).appendChild(&expr);
  }

  void onForEachStart() {/* TODO */}
  void onForEach(Token &out, Token &arr, Token &name, Token &value,
                 Token &stmt) {
    out.setNodeType(ONFOREACH).appendChild(&arr).appendChild(&name)
      .appendChild(&value).appendChild(&stmt);
  }

  void onTry(Token &out, Token &tryStmt, Token &className, Token &var,
             Token &catchStmt, Token &catches, Token &finallyStmt) {
    out.setNodeType(ONTRYCATCHFINALLY).appendChild(&tryStmt)
      .appendChild(&className).appendChild(&var).appendChild(&catchStmt)
      .appendChild(&catches).appendChild(&finallyStmt);
  }

  void onTry(Token &out, Token &tryStmt, Token &finallyStmt) {
    out.setNodeType(ONTRYFINALLY).appendChild(&tryStmt)
      .appendChild(&finallyStmt);
  }

  void onCatch(Token &out, Token &catches, Token &className, Token &var,
               Token &stmt) {
    out.setNodeType(ONCATCH).appendChild(&catches).appendChild(&className)
      .appendChild(&var).appendChild(&stmt);
  }

  void onFinally(Token &out, Token &stmt) {
    out.setNodeType(ONFINALLY).appendChild(&stmt);
  }

  void onThrow(Token &out, Token &expr) {
    out.setNodeType(ONTHROW).appendChild(&expr);
  }

  Token onClosure(ClosureType type,
                  Token* modifier,
                  Token& ref,
                  Token& params,
                  Token& cparams,
                  Token& stmts) {
    // TODO
    Token ret;
    return ret;
  }

  void onClosureParam(Token &out, Token *params, Token &param, bool ref) {
    out.setNodeType(ONCLOSUREPARAM).appendChild(params).appendChild(&param)
      .setExtra(new OnClosureParamEI(ref));
  }

  Token onExprForLambda(const Token&) { /* TODO */ Token ret; return ret; }

  void onLabel(Token &out, Token &label) {
    out.setNodeType(ONLABEL).appendChild(&label);
  }
  void onGoto(Token &out, Token &label, bool limited) {
    out.setNodeType(ONGOTO).appendChild(&label).setExtra(new OnGotoEI(limited));
  }
  void onTypedef(Token& out, Token& name, Token& value) {
    out.setNodeType(ONTYPEDEF).appendChild(&name).appendChild(&value);
  }
  void onTypeAnnotation(Token& out, const Token& name, const Token& typeArgs) {
    out.setNodeType(ONTYPEANNOTATION).appendChild(const_cast<Token *>(&name))
      .appendChild(const_cast<Token *>(&typeArgs));
  }
  void onTypeList(Token& type1, const Token& type2) {
    // TODO: no out?
//    out.setNodeType(ONTYPELIST).appendChild(const_cast<Token *>(&type2));
  }
  void onTypeSpecialization(Token& type, char specialization) {
    // TODO: no out?
//    out.setNodeType(ONTYPESPECIALIZATION)
//      .setExtra(new OnTypeSpecializationEI(specialization));
  }

  void onQuery(Token &out, Token &head, Token &body) {
    out.setNodeType(ONQUERY).appendChild(&head).appendChild(&body);
  }
  void onQueryBody(Token &out, Token *clauses, Token &select, Token *cont) {
    out.setNodeType(ONQUERYBODY).appendChild(clauses).appendChild(&select)
      .appendChild(cont);
  }
  void onQueryBodyClause(Token &out, Token *clauses, Token &clause) {
    out.setNodeType(ONQUERYBODYCLAUSE).appendChild(clauses)
      .appendChild(&clause);
  }
  void onFromClause(Token &out, Token &var, Token &coll) {
    out.setNodeType(ONFROMCLAUSE).appendChild(&var).appendChild(&coll);
  }
  void onLetClause(Token &out, Token &var, Token &expr) {
    out.setNodeType(ONLETCLAUSE).appendChild(&var).appendChild(&expr);
  }
  void onWhereClause(Token &out, Token &expr) {
    out.setNodeType(ONWHERECLAUSE).appendChild(&expr);
  }
  void onJoinClause(Token &out, Token &var, Token &coll, Token &left,
      Token &right) {
    out.setNodeType(ONJOINCLAUSE).appendChild(&var).appendChild(&coll)
      .appendChild(&left).appendChild(&right);
  }
  void onJoinIntoClause(Token &out, Token &var, Token &coll, Token &left,
      Token &right, Token &group) {
    out.setNodeType(ONJOININTOCLAUSE).appendChild(&var).appendChild(&coll)
      .appendChild(&left).appendChild(&right).appendChild(&group);
  }
  void onOrderbyClause(Token &out, Token &orderings) {
    out.setNodeType(ONORDERBYCLAUSE).appendChild(&orderings);
  }
  void onOrdering(Token &out, Token *orderings, Token &ordering) {
    out.setNodeType(ONORDERING).appendChild(orderings).appendChild(&ordering);
  }
  void onOrderingExpr(Token &out, Token &expr, Token *direction) {
    out.setNodeType(ONORDERINGEXPR).appendChild(&expr).appendChild(direction);
  }
  void onSelectClause(Token &out, Token &expr) {
    out.setNodeType(ONSELECTCLAUSE).appendChild(&expr);
  }
  void onGroupClause(Token &out, Token &coll, Token &key) {
    out.setNodeType(ONGROUPCLAUSE).appendChild(&coll).appendChild(&key);
  }
  void onIntoClause(Token &out, Token &var, Token &query) {
    out.setNodeType(ONINTOCLAUSE).appendChild(&var).appendChild(&query);
  }

  // for namespace support
  void onNamespaceStart(const std::string &ns, bool file_scope = false) {
    // TODO
  }
  void onNamespaceEnd() {}
  void onUse(const std::string &ns, const std::string &as) {
    // TODO
  }
  void nns(bool declare = false) {
    // TODO
  }
  std::string nsDecl(const std::string &name) {
    // TODO
    return name;
  }
  std::string resolve(const std::string &ns, bool cls) {
    // TODO
    return ns;
  }

  void onNewLabelScope(bool fresh) {}
  void onScopeLabel(const Token& stmt, const Token& label) {}
  void onCompleteLabelScope(bool fresh) {}


  /////////////////////////////////////////////////////////////////////////////
  // Functions below are used to output XHPAST
  /////////////////////////////////////////////////////////////////////////////

  void coalesceTree() {
    coalesceTreeImpl(&tree);
  }
  // Collapse/elide certain nodes to avoid deep trees
  void coalesceTreeImpl(Token *node) {
    if (node != nullptr) {
      // copy children
      std::vector<Token *> children1(node->children);
      node->children.clear();
      for (std::vector<Token *>::iterator i = children1.begin();
           i < children1.end();
           ++i) {
        Token *child = *i;
        if (child && child->nodeType == ONSTATEMENTLISTSTART) {
          delete child; // no longer in the tree
          continue;
        }
        coalesceTreeImpl(child);
        if (child && (child->nodeType == node->nodeType) &&
            ((child->nodeType == ADDSTATEMENT))) {
          node->children.insert(node->children.end(),
                                child->children.begin(),
                                child->children.end());
          delete child; // no longer in the tree
        } else {
          node->children.push_back(child);
        }
      }
    }
  }
  xhpast::Node* outputXHPAST() {
    xhpast::Node* xhpast_tree = outputXHPASTImpl(&tree);
    xhpast::Node* root = new xhpast::Node(n_PROGRAM);
    root->appendChild(xhpast_tree);
    root->l_tok = xhpast_tree->l_tok;
    return root;
  }
  xhpast::Node* extend_right(xhpast::Node *n, int type) {
    // Extend r_tok to the first token of type
    n->r_tok = scan_forward(n->r_tok + 1, type);
    return n;
  }
  xhpast::Node* extend_left(xhpast::Node *n, int type) {
    // Extend l_tok to the first token of type
    for (int i = n->l_tok - 1; i >= 0; i--) {
      xhpast::Token *t = m_listener.tokens[i];
      if (t->type == type) {
        n->l_tok = i;
        return n;
      }
    }
    // missing token!
    always_assert(false);
  }
  int scan_forward(int start, int type) {
    for (int i = start; i < m_listener.tokens.size(); i++) {
      if (m_listener.tokens[i]->type == type) {
        return i;
      }
    }
    // missing token!
    always_assert(false);
  }
  int scan_backward(int start, int type) {
    for (int i = start; i >= 0; i--) {
      if (m_listener.tokens[i]->type == type) {
        return i;
      }
    }
    // missing token!
    always_assert(false);
  }
  xhpast::Node* extend_to_delimiters(xhpast::Node *n, int left, int right) {
    extend_left(n, left);
    return extend_right(n, right);
  }
  // returns l_tok of first real child
  int transform_children(Token *node, xhpast::Node *n) {
    int l_tok = -2;
    for (std::vector<Token *>::iterator i = node->children.begin();
         i < node->children.end();
         ++i) {
      // TODO: It is possible that in all cases with NULL children, we just
      // want to append n_EMPTY, but just continue quietly for now.
      if (!(*i)) continue;
      xhpast::Node *newChild = outputXHPASTImpl(*i);
      n->appendChild(newChild);
      if (l_tok == -2) {
        l_tok = newChild->l_tok;
      }
      // optionally insert an n_OPEN_TAG if it follows an ONECHO
      // TODO: only for HTML?
      if ((*i)->nodeType == ONECHO) {
        int id = (*i)->ID() + 1;
        if (id < m_listener.tokens.size()) {
          xhpast::Token *t = m_listener.tokens[id];
          if (t->type == T_OPEN_TAG) {
            n->appendChild(new xhpast::Node(n_OPEN_TAG, id, id));
          }
        }
      }
    }
    return l_tok;
  }
  int insert_binary_operator(xhpast::Node *n) {
    // Locate operator and insert between children
    xhpast::Node *c1 = n->children.front();
    xhpast::Node *c2 = n->children.back();
    int id1 = c1->r_tok; // must be greater than this
    int id2 = c2->l_tok;  // and less than this
    for (int i = id1 + 1; i < id2; i++) {
      xhpast::Token *t = m_listener.tokens[i];
      switch (t->type) {
        case T_COMMENT:
        case T_DOC_COMMENT: // needed?
        case T_WHITESPACE: {
          continue;
        }
        default: {
          n->children.pop_back();
          n->children.push_back(new xhpast::Node(n_OPERATOR, i, i));
          n->children.push_back(c2);
          return t->type;
        }
      }
    }
    always_assert(false); // couldn't find the operator!
  }
  xhpast::Node* outputCondition(Token *cond) {
    xhpast::Node *control_cond = new xhpast::Node(n_CONTROL_CONDITION);
    control_cond->appendChild(outputXHPASTImpl(cond));
    extend_to_delimiters(control_cond, '(', ')');
    return control_cond;
  }
  xhpast::Node *childOf(xhpast::Node *child, int type) {
    xhpast::Node *parent = new xhpast::Node(type);
    parent->appendChild(child);
    return parent;
  }
  // parse ;, stmt;, {} or { stmts; }
  xhpast::Node *possibleStatements(int start, Token *node) {
    if (node->nodeType == RAW) {
      // We need to find the ";"
      int semicolon = scan_forward(start, ';');
      xhpast::Node *stmt = new xhpast::Node(n_STATEMENT, semicolon);
      stmt->appendChild(new xhpast::Node(n_EMPTY));
      return stmt;
    } else {
      return outputXHPASTImpl(node);
    }
  }
  xhpast::Node *declareParamList(Token *params, int name_pos) {
    xhpast::Node *param_list =
      new xhpast::Node(n_DECLARATION_PARAMETER_LIST);
    while (params && (params->nodeType == ONPARAM)) {
      std::vector<Token *>::iterator i = params->children.begin();
      params = *i++;
      UNUSED Token *type = *i++; // TODO
      Token *var = *i++;
      UNUSED Token *defValue = *i++; // TODO
      UNUSED Token *attr = *i++; // TODO
      UNUSED Token *mods = *i; // TODO
      // TODO: bool ref
      xhpast::Node *p = new xhpast::Node(n_DECLARATION_PARAMETER);
      // TODO: class type
      p->appendChild(new xhpast::Node(n_EMPTY));
      p->appendChild(new xhpast::Node(n_VARIABLE, var->ID()));
      // TODO: default value
      p->appendChild(new xhpast::Node(n_EMPTY));
      // Must prepend to get argument order right
      param_list->prependChild(p);
    }
    if (param_list->children.size() == 0) {
      // no params
      param_list->l_tok = scan_forward(name_pos, '(');
      param_list->r_tok = scan_forward(param_list->l_tok, ')');
    } else {
      extend_to_delimiters(param_list, '(', ')');
    }
    return param_list;
  }
  xhpast::Node *statementList(Token *stmt, int last_loc) {
    // statement list: need to extend this to brackets
    if (stmt) {
      xhpast::Node *stmts = outputXHPASTImpl(stmt);
      if (stmt->nodeType != FINISHSTATEMENT) {
        extend_to_delimiters(stmts, '{', '}');
      }
      return stmts;
    } else {
      // empty body; start looking for braces after the params
      int open_brace = scan_forward(last_loc, '{');
      xhpast::Node *braces = new xhpast::Node(n_STATEMENT_LIST,
                                              open_brace);
      extend_right(braces, '}');
      return braces;
    }
  }
  xhpast::Node *callParamList(Token *params, int name_pos) {
    xhpast::Node *param_list = new xhpast::Node(n_CALL_PARAMETER_LIST);
    while (params && (params->nodeType == ONCALLPARAM)) {
      std::vector<Token *>::iterator i = params->children.begin();
      params = *i++;
      // TODO: ref
      param_list->prependChild(outputXHPASTImpl(*i));
    }
    if (param_list->children.size() == 0) {
      // no params
      param_list->l_tok = scan_forward(name_pos, '(');
      param_list->r_tok = scan_forward(param_list->l_tok, ')');
    } else {
      extend_to_delimiters(param_list, '(', ')');
    }
    return param_list;
  }
  xhpast::Node *objectProperty(Token *base, Token *prop) {
    xhpast::Node *arrow = new xhpast::Node(n_OBJECT_PROPERTY_ACCESS);
    arrow->appendChild(outputXHPASTImpl(base));
    arrow->appendChild(new xhpast::Node(n_STRING, prop->ID()));
    return arrow;
  }
  xhpast::Node *outputXHPASTImpl(Token *node) {
   xhpast::Node* n = new xhpast::Node();
    n->l_tok = node->ID();
    n->r_tok = node->ID();

    switch (node->nodeType) {
      case ONSIMPLEVARIABLE: {
        // no children
        n->type = n_VARIABLE;
        break;
      }
      case ONECHO: {
        std::vector<Token *>::iterator i = node->children.begin();
        if ((*i)->nodeType == RAW) {
          // no children
          n->type = n_INLINE_HTML;
        } else {
          n->type = n_ECHO_LIST;
          Token *current = *i;
          while (current) {
            std::vector<Token *>::iterator i = current->children.begin();
            current = *i++;
            xhpast::Node *child = outputXHPASTImpl(*i);
            if (n->r_tok < child->r_tok) {
              n->r_tok = child->r_tok;
            }
            n->prependChild(child);
          }
          xhpast::Node *stmt = childOf(n, n_STATEMENT);
          return extend_right(stmt, ';');
        }
        break;
      }
      case ONSTATEMENTLISTSTART: {
        // Should never reach here because parent should have elided this
        always_assert(false);
        break;
      }
      case ONEXPSTATEMENT: {
        n->type = n_STATEMENT;
        transform_children(node, n);
        extend_right(n, ';');
        break;
      }
      case ADDSTATEMENT: {
        n->type = n_STATEMENT_LIST;
        // ADDSTATEMENT simply contains its children, so set l_tok to be l_tok
        // of first child; r_tok will be set correctly by transform_children
        n->l_tok = transform_children(node, n);
        break;
      }
      case ONRETURN: {
        n->type = n_RETURN;
        std::vector<Token *>::iterator i = node->children.begin();
        Token *expr = *i;
        if (expr) {
          transform_children(node, n);
        } else {
          n->appendChild(new xhpast::Node(n_EMPTY));
        }
        xhpast::Node *stmt = childOf(n, n_STATEMENT);
        return extend_right(stmt, ';');
      }
      case ONMETHOD: {
        std::vector<Token *>::iterator i = node->children.begin();
        Token *modifiers = *i++;
        UNUSED Token *ret = *i++; // TODO
        UNUSED Token *ref = *i++; // TODO
        Token *name = *i++;
        Token *params = *i++;
        Token *stmt = *i++;
        UNUSED Token *attr = *i; // TODO
        // TODO: reloc
        n->type = n_METHOD_DECLARATION;
        // modifiers private public protected, etc.
        xhpast::Node *mods = new xhpast::Node(n_METHOD_MODIFIER_LIST);
        while (modifiers && (modifiers->nodeType == ONMEMBERMODIFIER)) {
          i = modifiers->children.begin();
          modifiers = *i++;
          mods->appendChild(new xhpast::Node(n_STRING, (*i)->ID()));
        }
        n->appendChild(mods);
        // TODO
        n->appendChild(new xhpast::Node(n_EMPTY));
        n->appendChild(new xhpast::Node(n_STRING, name->ID()));
        xhpast::Node *param_list = declareParamList(params, name->ID());
        n->appendChild(param_list);
        // TODO
        n->appendChild(new xhpast::Node(n_EMPTY));
        n->appendChild(statementList(stmt, param_list->r_tok));
        // Hang method off n_statement
        return childOf(n, n_STATEMENT);
      }
      case ONFUNCTION: {
        // children are modifiers (maybe null), ret, ref, name, params, stmt,
        // attr (maybe null)
        std::vector<Token *>::iterator i = node->children.begin();
        UNUSED Token *modifiers = *i++; // TODO
        UNUSED Token *ret = *i++; // TODO
        UNUSED Token *ref = *i++; // TODO
        Token *name = *i++;
        Token *params = *i++;
        Token *stmt = *i++;
        UNUSED Token *attr = *i; // TODO
        n->type = n_FUNCTION_DECLARATION;
        // TODO: T_STATIC
        n->appendChild(new xhpast::Node(n_EMPTY));
        // TODO: is_reference
        n->appendChild(new xhpast::Node(n_EMPTY));
        // function name
        n->appendChild(new xhpast::Node(n_STRING, name->ID()));
        // params
        xhpast::Node *param_list = declareParamList(params, name->ID());
        n->appendChild(param_list);
        // lexical vars
        n->appendChild(new xhpast::Node(n_EMPTY));
        // statement list: need to extend this to brackets
        n->appendChild(statementList(stmt, param_list->r_tok));
        // Hang function off n_statement
        return childOf(n, n_STATEMENT);
      }
      case ONPARAM: {
        // should be handled in onfunction / onmethod
        always_assert(false);
        break;
      }
      case ONASSIGN:
      case ONBINARYOPEXP: {
        n->type = n_BINARY_EXPRESSION;
        transform_children(node, n);
        int t = insert_binary_operator(n);
        // See if n_CONCATENATION_LIST
        if (t == '.') {
          n->type = n_CONCATENATION_LIST;
          xhpast::Node *second = n->children.back();
          n->children.pop_back();
          xhpast::Node *op = n->children.back();
          n->children.pop_back();
          xhpast::Node *first = n->children.back();
          n->children.pop_back();
          if (first->type == n_CONCATENATION_LIST) {
            n->appendChildren(first);
          } else {
            n->appendChild(first);
          }
          n->appendChild(op);
          // note: the second child should never be an n_CONCATENATION_LIST due
          // to the structure of the parse tree
          n->appendChild(second);
        }
        break;
      }
      case ONIF: {
        // children are cond stmt elsifs elsestmt
        n->type = n_IF;
        std::vector<Token *>::iterator i = node->children.begin();
        Token *cond = *i++;
        Token *stmt = *i++;
        Token *elseifs = *i++;
        Token *elsestmt = *i;
        n->type = n_IF;
        // condition
        n->appendChild(outputCondition(cond));
        // statements
        n->appendChild(outputXHPASTImpl(stmt));
        // elseifs
        while (elseifs->nodeType == ONELSEIF) {
          i = elseifs->children.begin();
          Token *next_elseifs = *i++;
          xhpast::Node *elseif_out = new xhpast::Node(n_ELSEIF);
          elseif_out->appendChild(outputCondition(*i++));
          elseif_out->appendChild(outputXHPASTImpl(*i));
          extend_left(elseif_out, T_ELSEIF);
          n->appendChild(elseif_out);
          elseifs = next_elseifs;
        }
        xhpast::Node *else_out = new xhpast::Node(n_ELSE);
        else_out->appendChild(outputXHPASTImpl(elsestmt));
        extend_left(else_out, T_ELSE);
        n->appendChild(else_out);

        xhpast::Node *cond_list = new xhpast::Node(n_CONDITION_LIST);
        cond_list->appendChild(n);
        return childOf(cond_list, n_STATEMENT);
      }
      case ONWHILE: {
        n->type = n_WHILE;
        std::vector<Token *>::iterator i = node->children.begin();
        n->appendChild(outputCondition(*i++));
        n->appendChild(possibleStatements(n->r_tok, *i));
        return childOf(n, n_STATEMENT);
      }
      case ONFOR: {
        n->type = n_FOR;
        std::vector<Token *>::iterator i = node->children.begin();
        // initialization
        xhpast::Node *loop_exprs = childOf(outputXHPASTImpl(*i++),
                                           n_FOR_EXPRESSION);
        // termination condition
        loop_exprs->appendChild(outputXHPASTImpl(*i++));
        // loop update
        loop_exprs->appendChild(outputXHPASTImpl(*i++));
        extend_to_delimiters(loop_exprs, '(', ')');
        n->appendChild(loop_exprs);
        // loop body
        n->appendChild(possibleStatements(n->r_tok, *i));
        return childOf(n, n_STATEMENT);
      }
      case ONEXPRLISTELEM: {
        transform_children(node, n);
        n->type = n_EXPRESSION_LIST;
        break;
      }
      case ONBLOCK: {
        transform_children(node, n);
        if (node->children.size() > 0) {
          xhpast::Node *stmts = n->firstChild();
          extend_to_delimiters(stmts, '{', '}');
          return stmts;
        } else {
          n->type = n_STATEMENT_LIST;
          extend_right(n, '}');
        }
        break;
      }
      case ONSCALAR: {
        OnScalarEI *ei = dynamic_cast<OnScalarEI*>(node->extra);
        switch (ei->type) {
          case T_DNUMBER:
          case T_LNUMBER: {
            n->type = n_NUMERIC_SCALAR;
            break;
          }
          case T_CONSTANT_ENCAPSED_STRING: {
            n->type = n_STRING_SCALAR;
            break;
          }
          case T_LINE:
          case T_FILE:
          case T_DIR:
          case T_CLASS_C:
          case T_TRAIT_C:
          case T_METHOD_C:
          case T_FUNC_C:
          case T_NS_C: {
            n->type = n_MAGIC_SCALAR;
            break;
          }
          default: {
            // where do we output n_HEREDOC?
            always_assert(false); // unexpected
          }
        }
        break;
      }
      case ONUNARYOPEXP: {
        transform_children(node, n);
        OnUnaryOpExpEI *ei = dynamic_cast<OnUnaryOpExpEI*>(node->extra);
        if (ei->front) {
          n->type = n_UNARY_PREFIX_EXPRESSION;
          int loc = scan_backward(node->ID(), ei->op);
          n->prependChild(new xhpast::Node(n_OPERATOR, loc));
        } else { // back
          n->type = n_UNARY_POSTFIX_EXPRESSION;
          int loc = scan_forward(node->ID(), ei->op);
          n->appendChild(new xhpast::Node(n_OPERATOR, loc));
        }
        break;
      }
      case ONBREAK: {
        n->type = n_BREAK;
        // TODO: break expr
        n->appendChild(new xhpast::Node(n_EMPTY));
        xhpast::Node* top_stmt = childOf(n, n_STATEMENT);
        return extend_right(top_stmt, ';');
      }
      case ONCONTINUE: {
        n->type = n_CONTINUE;
        // TODO: continue expr
        n->appendChild(new xhpast::Node(n_EMPTY));
        xhpast::Node* top_stmt = childOf(n, n_STATEMENT);
        return extend_right(top_stmt, ';');
      }
      case ONSWITCH: {
        n->type = n_SWITCH;
        std::vector<Token *>::iterator i = node->children.begin();
        // first child is control condition
        n->appendChild(outputCondition(*i++));
        // next child is list of cases
        xhpast::Node *case_list = new xhpast::Node(n_STATEMENT_LIST);
        int right_most = n->r_tok;
        Token* cases = *i;
        while (cases->nodeType == ONCASE) {
          i = cases->children.begin();
          std::vector<Token *>::iterator end = cases->children.end();
          cases = *i++;
          // cse because case is a reserved word
          xhpast::Node *cse;
          if (*i) {
            cse = new xhpast::Node(n_CASE);
            // condition
            cse->appendChild(outputXHPASTImpl(*i));
            extend_left(cse, T_CASE);
          } else {
            std::cout << right_most << std::endl;
            int loc = scan_forward(right_most, T_DEFAULT);
            cse = new xhpast::Node(n_DEFAULT, loc);
          }
          if (i != end) {
            i++;
          }
          // body
          if (i != end) {
            // TODO: replace with possibleStatements?
            cse->appendChild(outputXHPASTImpl(*i));
          } else {
            cse->appendChild(new xhpast::Node(n_STATEMENT_LIST));
          }
          // must prepend to get the order right
          case_list->prependChild(cse);
          right_most = case_list->r_tok;
        }
        n->appendChild(extend_to_delimiters(case_list, '{', '}'));
        return childOf(n, n_STATEMENT);
      }
      case ONARRAY: {
        // TODO: depends on type
        n->type = n_ARRAY_LITERAL;
        xhpast::Node *value_list = new xhpast::Node(n_ARRAY_VALUE_LIST);
        std::vector<Token *>::iterator i = node->children.begin();
        Token *pair = *i;
        while (pair && pair->nodeType == ONARRAYPAIR) {
          // TODO: refs
          i = pair->children.begin();
          Token *next_pair = *i++;
          Token *name = *i++;
          Token *value = *i;
          xhpast::Node *entry = new xhpast::Node(n_ARRAY_VALUE);
          if (name) {
            entry->appendChild(outputXHPASTImpl(name));
          } else {
            entry->appendChild(new xhpast::Node(n_EMPTY));
          }
          entry->appendChild(outputXHPASTImpl(value));
          // must prepend to get the order right
          value_list->prependChild(entry);
          pair = next_pair;
        }
        n->appendChild(value_list);
        extend_right(n, ')');
        break;
      }
      case ONOBJECTMETHODCALL: {
        std::vector<Token *>::iterator i = node->children.begin();
        Token *base = *i++;
        Token *prop = *i++;
        Token *params = *i;
        n->type = n_METHOD_CALL;
        n->appendChild(objectProperty(base, prop));
        n->appendChild(callParamList(params, prop->ID()));
        break;
      }
      case ONCALL: {
        std::vector<Token *>::iterator i = node->children.begin();
        Token *name = *i++;
        Token *params = *i++;
        UNUSED Token *cls = *i; // TODO
        // TODO: dynamic, fromCompiler, cls
        n->type = n_FUNCTION_CALL;
        n->appendChild(new xhpast::Node(n_SYMBOL_NAME, name->ID()));
        n->appendChild(callParamList(params, name->ID()));
        break;
      }
      case ONCLASS: {
        std::vector<Token *>::iterator i = node->children.begin();
        Token *name = *i++;
        UNUSED Token *base = *i++; // TODO
        UNUSED Token *baseInterface = *i++; // TODO
        Token *stmt = *i++;
        UNUSED Token *attr = *i; // TODO
        n->type = n_CLASS_DECLARATION;
        // TODO attributes
        xhpast::Node *attr_out = new xhpast::Node(n_CLASS_ATTRIBUTES, n->l_tok);
        attr_out->appendChild(new xhpast::Node(n_EMPTY));
        n->appendChild(attr_out);
        n->appendChild(new xhpast::Node(n_CLASS_NAME, name->ID()));
        // TODO
        n->appendChild(new xhpast::Node(n_EMPTY));
        // TODO
        n->appendChild(new xhpast::Node(n_EMPTY));
        n->appendChild(outputXHPASTImpl(stmt));
        return childOf(n, n_STATEMENT);
      }
      case ONCLASSSTATEMENT: {
        n->type = n_STATEMENT_LIST;
        Token *stmts = node;
        do {
          std::vector<Token *>::iterator i = stmts->children.begin();
          stmts = *i++;
          Token *stmt = *i;
          n->prependChild(outputXHPASTImpl(stmt));
        } while(stmts->nodeType == ONCLASSSTATEMENT);
        extend_to_delimiters(n, '{', '}');
        break;
      }
      case FINISHSTATEMENT: {
        if (node->children.size() > 0) {
          std::vector<Token *>::iterator i = node->children.begin();
          return extend_to_delimiters(outputXHPASTImpl(*i), '{', '}');
        } else {
          n->type = n_STATEMENT_LIST;
          return extend_right(n, '}');
        }
      }
      case ONDO: {
        n->type = n_DO_WHILE;
        std::vector<Token *>::iterator i = node->children.begin();
        n->appendChild(possibleStatements(n->l_tok, *i++));
        n->appendChild(outputCondition(*i));
        xhpast::Node *ret = childOf(n, n_STATEMENT);
        return extend_right(ret, ';');
      }
      case ONCONSTANTVALUE: {
        n->type = n_SYMBOL_NAME;
        break;
      }
      case ONENCAPSLIST: {
        transform_children(node, n);
        n->type = n_STRING_SCALAR;
        n->children.clear(); // memory leak, + remember to collapse range to 318
        break;
      }
      case ONOBJECTPROPERTY: {
        std::vector<Token *>::iterator i = node->children.begin();
        Token *base = *i++;
        Token *prop = *i;
        return objectProperty(base, prop);
      }
      case ONYIELD: {
        transform_children(node, n);
        n->type = n_YIELD_EXPRESSION;
        xhpast::Node *yield_kw = new xhpast::Node(n_YIELD, node->ID());
        n->prependChild(yield_kw);
        break;
      }
      case ONAWAIT: {
        transform_children(node, n);
        n->type = n_AWAIT_EXPRESSION;
        xhpast::Node *await_kw = new xhpast::Node(n_AWAIT, node->ID());
        n->prependChild(await_kw);
        break;
      }
      default: {
        transform_children(node, n);
        n->type = node->nodeType;
        break;
      }
    }
    // fyi: Some branches of the switch return early
    return n;
  }
};

//////////////////////////////////////////////////////////////////////

}}

#endif

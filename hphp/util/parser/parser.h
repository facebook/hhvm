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

#ifndef incl_HPHP_UTIL_PARSER_PARSER_H_
#define incl_HPHP_UTIL_PARSER_PARSER_H_

#include "hphp/util/parser/scanner.h"
#include "hphp/util/lock.h"
#include "hphp/util/case_insensitive.h"

#define IMPLEMENT_XHP_ATTRIBUTES                \
  Token m_xhpAttributes;                        \
  Token *xhpGetAttributes() {                   \
    if (m_xhpAttributes.num()) {                \
      return &m_xhpAttributes;                  \
    }                                           \
    return nullptr;                                \
  }                                             \
  void xhpSetAttributes(Token &t) {             \
    m_xhpAttributes = t;                        \
    m_xhpAttributes = 1;                        \
  }                                             \
  void xhpResetAttributes() {                   \
    m_xhpAttributes.reset();                    \
  }                                             \

// NOTE: system/classes/closure.php may have reference to these strings:
#define CONTINUATION_OBJECT_NAME       "__cont__"
#define TYPED_CONTINUATION_OBJECT_NAME "__typedCont__"
#define FOREACH_VAR_PREFIX             "__foreach__"
#define NAMESPACE_SEP                  '\\'

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

typedef void * TStatementPtr;
class ParserBase {
public:
  enum NameKind {
    StringName,
    VarName,
    ExprName,
    StaticClassExprName,
    StaticName
  };

  static bool IsClosureName                (const std::string &name);
  static bool IsContinuationName           (const std::string &name);
  static bool IsClosureOrContinuationName  (const std::string &name);
  static std::string newContinuationName   (const std::string &name);
  std::string newClosureName(
      const std::string &className,
      const std::string &funcName);

public:
  ParserBase(Scanner &scanner, const char *fileName);
  virtual ~ParserBase();

  Scanner &scanner() { return m_scanner;}

  /**
   * Main function to call to start parsing the file. This function is already
   * implemented in hphp.y. Therefore, a subclass only has to declare it.
   */
  virtual bool parseImpl() = 0;

  /**
   * Raise a parser error.
   */
  virtual void error(const char* fmt, ...) = 0;

  /**
   * How to decide whether to turn on XHP.
   */
  virtual bool enableXHP() = 0;

  /**
   * Public accessors.
   */
  const char *file() const { return m_fileName;}
  std::string getMessage(bool filename = false) const;
  std::string getMessage(Location *loc, bool filename = false) const;
  LocationPtr getLocation() const;
  void getLocation(Location &loc) const {
    loc = m_loc;
    loc.file = file();
  }

  int line0() const { return m_loc.line0;}
  int char0() const { return m_loc.char0;}
  int line1() const { return m_loc.line1;}
  int char1() const { return m_loc.char1;}
  int cursor() const { return m_loc.cursor;}

  // called by generated code
  int scan(ScannerToken *token, Location *loc) {
    return m_scanner.getNextToken(*token, *loc);
  }
  void setRuleLocation(Location *loc) {
    m_loc = *loc;
  }
  virtual void fatal(Location *loc, const char *msg) {}

  void pushFuncLocation();
  LocationPtr popFuncLocation();
  void pushClass(bool isXhpClass);
  bool peekClass();
  void popClass();

  // for typevar checking
  void pushTypeScope();
  void popTypeScope();
  void addTypeVar(const std::string &name);
  bool isTypeVar(const std::string &name);

  // for goto syntax checking
  void pushLabelInfo();
  void pushLabelScope();
  void popLabelScope();
  void addLabel(const std::string &label, LocationPtr loc,
                ScannerToken *stmt);
  void addGoto(const std::string &label, LocationPtr loc,
               ScannerToken *stmt);
  void popLabelInfo();

  enum GotoError {
    UndefLabel = 1,
    InvalidBlock,
  };

  virtual void invalidateGoto(TStatementPtr expr, GotoError error) = 0;
  virtual void invalidateLabel(TStatementPtr expr) = 0;

  virtual TStatementPtr extractStatement(ScannerToken *stmt) = 0;

protected:
  Scanner &m_scanner;
  const char *m_fileName;

  Location m_loc;
  LocationPtrVec m_funcLocs;
  std::vector<bool> m_classes; // used to determine if we are currently
                               // inside a regular class or an XHP class

  struct LabelStmtInfo {
    int scopeId;
    TStatementPtr stmt;
    bool inTryCatchBlock;
    LocationPtr loc;
  };
  typedef std::map<std::string, LabelStmtInfo> LabelMap;
    // name => LabelStmtInfo

  typedef std::set<std::string> TypevarScope;
  typedef std::vector<TypevarScope> TypevarScopeStack;
  TypevarScope m_typeVars;
  TypevarScopeStack m_typeScopes;

  // for goto syntax checking
  typedef std::vector<int> LabelScopes;
  struct GotoInfo {
    std::string label;
    LabelScopes scopes;
    LocationPtr loc;
    TStatementPtr stmt;
  };

  class LabelInfo {
  public:
    LabelInfo() : scopeId(0) {}
    int scopeId;
    LabelScopes scopes;
    LabelMap labels;
    std::vector<GotoInfo> gotos;
  };
  std::vector<LabelInfo> m_labelInfos; // stack by function

  // for namespace support
  enum NamespaceState {
    SeenNothing,
    SeenNonNamespaceStatement,
    SeenNamespaceStatement,
    InsideNamespace,
  };
  NamespaceState m_nsState;
  bool m_nsFileScope;
  std::string m_namespace; // current namespace
  hphp_string_imap<std::string> m_aliases;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_UTIL_PARSER_PARSER_H_

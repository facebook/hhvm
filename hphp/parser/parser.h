/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_PARSER_PARSER_H_
#define incl_HPHP_PARSER_PARSER_H_

#include <map>
#include <set>
#include <vector>
#include <string>
#include <memory>

#include "hphp/parser/scanner.h"
#include "hphp/util/lock.h"
#include "hphp/util/functional.h"
#include "hphp/util/hash-map.h"

#include <folly/String.h>

#define IMPLEMENT_XHP_ATTRIBUTES                \
  Token m_xhpAttributes;                        \
  Token *xhpGetAttributes() {                   \
    if (m_xhpAttributes.num()) {                \
      return &m_xhpAttributes;                  \
    }                                           \
    return nullptr;                             \
  }                                             \
  void xhpSetAttributes(Token &t) {             \
    m_xhpAttributes = t;                        \
    m_xhpAttributes = 1;                        \
  }                                             \
  void xhpResetAttributes() {                   \
    m_xhpAttributes.reset();                    \
  }                                             \

#define NAMESPACE_SEP                  '\\'

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * HHVM supports multiple types of lambda expressions.
 */
enum class ClosureType {
  /*
   * Short = Lambda syntax. Automatically captures variables mentioned in the
   * body.
   */
  Short,

  /*
   * Long = Traditional closure syntax. Only captures variables that are
   * explicitly specified in the "use" list.
   */
  Long,
};

enum class PropAccessType {
  Normal,
  NullSafe,
};

enum ObjPropType {
  ObjPropNormal,
  ObjPropXhpAttr
};

//////////////////////////////////////////////////////////////////////

typedef void* TStatementPtr;

struct ParserBase {
  enum NameKind {
    StringName,
    VarName,
    ExprName,
    StaticClassExprName,
    StaticName
  };

  enum class LabelScopeKind {
    Invalid,
    LoopSwitch,
    Finally,
    Using,
  };

  static bool IsClosureName(const std::string &name);
  /*
   * Is this the name of an anonymous class (either a closure,
   * or a ClassExpression).
   */
  static bool IsAnonymousClassName(folly::StringPiece name);

  static const char* labelScopeName(LabelScopeKind kind);

  std::string newClosureName(
      const std::string &namespaceName,
      const std::string &className,
      const std::string &funcName);
  std::string newAnonClassName(
      const std::string &prefix,
      const std::string &namespaceName,
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
  virtual void error(ATTRIBUTE_PRINTF_STRING const char* fmt, ...)
    ATTRIBUTE_PRINTF(2,3) = 0;

  /**
   * Public accessors.
   */
  const char *file() const { return m_fileName;}
  std::string getMessage(bool filename = false,
                         bool rawPosWhenNoError = false) const;
  std::string getMessage(const Location::Range& loc,
                         bool filename = false) const;
  const Location::Range& getRange() const;

  int line0() const { return m_loc.r.line0;}
  int char0() const { return m_loc.r.char0;}
  int line1() const { return m_loc.r.line1;}
  int char1() const { return m_loc.r.char1;}
  int cursor() const { return m_loc.cursor;}

  // called by generated code
  int scan(ScannerToken *token, Location *loc) {
    return m_scanner.getNextToken(*token, *loc);
  }
  void setRuleLocation(Location *loc) {
    m_loc = *loc;
  }
  virtual void fatal(const Location* /*loc*/, const char* /*msg*/) {}
  virtual void parseFatal(const Location* /*loc*/, const char* /*msg*/) {}

  void pushFuncLocation();
  Location::Range popFuncLocation();
  void pushClass(bool isXhpClass);
  bool peekClass();
  void popClass();

  // for typevar checking
  void pushTypeScope();
  void popTypeScope();
  void addTypeVar(const std::string &name);
  bool isTypeVar(const std::string &name);
  bool isTypeVarInImmediateScope(const std::string &name);

  // for goto syntax checking
  void pushLabelInfo();
  void pushLabelScope(LabelScopeKind kind);
  void popLabelScope();
  void addLabel(const std::string &label, const Location::Range& loc,
                ScannerToken *stmt);
  void addGoto(const std::string &label, const Location::Range& loc,
               ScannerToken *stmt);
  void popLabelInfo();

  enum GotoError {
    UndefLabel = 1,
    InvalidBlock,
  };

  virtual TStatementPtr extractStatement(ScannerToken *stmt) = 0;

protected:
  Scanner &m_scanner;
  const char *m_fileName;

  Location m_loc;
  std::vector<Location::Range> m_funcLocs;
  std::vector<bool> m_classes; // used to determine if we are currently
                               // inside a regular class or an XHP class

  struct LabelScopeInfo {
    LabelScopeInfo(LabelScopeKind kind, int id) : kind(kind), id(id) {}

    LabelScopeKind kind;
    int id;
  };

  struct LabelStmtInfo {
    TStatementPtr stmt;
    LabelScopeInfo scopeInfo;
    Location::Range loc;
    bool inTryCatchBlock;
  };
  typedef std::map<std::string, LabelStmtInfo> LabelMap;
    // name => LabelStmtInfo

  typedef std::set<std::string> TypevarScope;
  typedef std::vector<TypevarScope> TypevarScopeStack;
  TypevarScope m_typeVars;
  TypevarScopeStack m_typeScopes;

  // for goto syntax checking
  using LabelScopes = std::vector<LabelScopeInfo>;
  struct GotoInfo {
    std::string label;
    LabelScopes scopes;
    Location::Range loc;
    TStatementPtr stmt;
  };

  struct LabelInfo {
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
  hphp_string_imap<int> m_seenAnonClasses;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PARSER_PARSER_H_

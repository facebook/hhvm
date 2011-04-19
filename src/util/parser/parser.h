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

#ifndef __HPHP_UTIL_PARSER_PARSER_H__
#define __HPHP_UTIL_PARSER_PARSER_H__

#include <util/parser/scanner.h>
#include <util/lock.h>
#include <util/case_insensitive.h>

#define IMPLEMENT_XHP_ATTRIBUTES                \
  Token m_xhpAttributes;                        \
  Token *xhpGetAttributes() {                   \
    if (m_xhpAttributes.num()) {                \
      return &m_xhpAttributes;                  \
    }                                           \
    return NULL;                                \
  }                                             \
  void xhpSetAttributes(Token &t) {             \
    m_xhpAttributes = t;                        \
    m_xhpAttributes = 1;                        \
  }                                             \
  void xhpResetAttributes() {                   \
    m_xhpAttributes.reset();                    \
  }                                             \

// NOTE: system/classes/closure.php may have reference to these strings:
#define CONTINUATION_OBJECT_NAME "__cont__"
#define YIELD_LABEL_PREFIX "__yield__"
#define FOREACH_VAR_PREFIX "__foreach__"
#define NAMESPACE_SEP '\\'

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ParserBase {
public:
  enum NameKind {
    StringName,
    VarName,
    ExprName,
    StaticClassExprName,
    StaticName
  };

  /**
   * Reset parser static variables. Good for unit tests.
   */
  static void Reset();

public:
  ParserBase(Scanner &scanner, const char *fileName);
  virtual ~ParserBase();

  Scanner &scanner() { return m_scanner;}

  /**
   * Main function to call to start parsing the file. This function is already
   * implemented in hphp.y. Therefore, a subclass only has to declare it.
   */
  virtual bool parse() = 0;

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
    loc = *m_loc;
    loc.file = file();
  }

  int line0() const { return m_loc->line0;}
  int char0() const { return m_loc->char0;}
  int line1() const { return m_loc->line1;}
  int char1() const { return m_loc->char1;}

  // called by generated code
  int scan(ScannerToken *token, Location *loc) {
    return m_scanner.getNextToken(*token, *loc);
  }
  void setRuleLocation(Location *loc) {
    m_loc = loc;
  }
  void fatal(Location *loc, ParserBase *parser, const char *msg) {}

  void pushFuncLocation();
  LocationPtr popFuncLocation();
  std::string getClosureName();

  // for goto syntax checking
  void pushLabelInfo();
  void pushLabelScope();
  void popLabelScope();
  void addLabel(const std::string &label);
  void addGoto(const std::string &label, LocationPtr loc);
  void popLabelInfo();

  // for namespace support
  void onNamespaceStart(const std::string &ns);
  void onNamespaceEnd();
  void onUse(const std::string &ns, const std::string &as);
  void nns(bool declare = false);
  std::string nsDecl(const std::string &name);
  std::string resolve(const std::string &ns, bool cls);

protected:
  Scanner &m_scanner;
  const char *m_fileName;

  Location *m_loc;
  LocationPtrVec m_funcLocs;

  // for goto syntax checking
  typedef std::vector<int> LabelScopes;
  typedef std::map<std::string, int> LabelMap; // name => scopeId
  struct GotoInfo {
    std::string label;
    LabelScopes scopes;
    LocationPtr loc;
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
  std::string m_namespace; // current namespace
  hphp_string_imap<std::string> m_aliases;

  // for closure hidden name
  static Mutex s_mutex;
  static std::map<int64, int> s_closureIds;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_UTIL_PARSER_PARSER_H__

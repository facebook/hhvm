/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_CODE_GENERATOR_H_
#define incl_HPHP_CODE_GENERATOR_H_

#include <deque>
#include <map>
#include <ostream>
#include <set>
#include <utility>
#include <vector>

#include "hphp/util/deprecated/declare-boost-types.h"

#include "hphp/compiler/hphp.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(Construct);
DECLARE_BOOST_TYPES(BlockScope);
DECLARE_EXTENDED_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(FileScope);
DECLARE_BOOST_TYPES(LoopStatement);

class CodeGenerator {
public:
  enum Output {
    InvalidOutput,

    PickledPHP, // stripped comments, etc. but 1 to 1 file to file
    InlinedPHP, // all includes are inlined
    TrimmedPHP, // unreferenced functions and classes are removed
    MonoCPP,    // All in one file. Left in for test. Not fully correct.
    FileCPP,    // 1 to 1 from php to cpp file
    ClusterCPP, // each directory up to a certain depth to a cpp file
    SystemCPP,  // special mode for generating builtin classes
    TextHHBC,   // HHBC dump in human-readable format
    BinaryHHBC, // serialized HHBC
  };

  enum Stream {
    NullStream = -1,    // suppress output
    PrimaryStream,      // main output
    ImplFile,           // C++ implementation file
    FatFile = ImplFile, // trimmed functions and classes
    MapFile,            // lineno mapping between trimmed and original

    StreamCount
  };

  enum Context {
    NoContext,

    CppForwardDeclaration,
    CppDeclaration,             // functions and classes
    CppImplementation,          // other statements than declarations
    CppPseudoMain,              // pseudo mains
    CppConstructor,             // we are generating class constructor
    CppInitializer,             // we are generating class initializer
    CppClassConstantsDecl,
    CppClassConstantsImpl,
    CppConstantsDecl,           // we are generating constant declarations
    CppFunctionWrapperImpl,     // Only used to force parameters to use C*Ref
    CppFunctionWrapperDecl,
    CppParameterDefaultValueDecl,
    CppParameterDefaultValueImpl,
    CppTypedParamsWrapperImpl,
    CppTypedParamsWrapperDecl,
    CppFFIDecl,
    CppFFIImpl,
    HsFFI,
    JavaFFI,
    JavaFFIInterface,           // for translating interfaces
    JavaFFICppDecl,             // javah is too slow, generate .h ourselves
    JavaFFICppImpl,
    SwigFFIDecl,
    SwigFFIImpl,

    PhpDeclaration,
    PhpImplementation,
  };

  enum BreakScopeBit {
    InsideSwitch = 0x80000000,
    StaticCases  = 0x40000000,
    BreakScopeBitMask = InsideSwitch | StaticCases
  };

public:
  CodeGenerator() {} // only for creating a dummy code generator
  explicit CodeGenerator(std::ostream *primary, Output output = PickledPHP,
                         const std::string *filename = nullptr);

  /**
   * ...if it was passed in from constructor.
   */
  const std::string& getFileName() const { return m_filename;}

  /**
   * What kind of program are we generating?
   */
  Output getOutput() const { return m_output;}
  void setOutput(Output output) { m_output = output;}

  /**
   * Stream functions.
   */
  void useStream(Stream stream);
  bool usingStream(Stream stream);
  std::ostream *getStream() const { return m_out;}
  std::ostream *getStream(Stream stream) const;
  void setStream(Stream stream, std::ostream *out);

  /**
   * Output strings.
   */
  void printf(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
    ATTRIBUTE_PRINTF(2,3);
  void indentBegin(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
    ATTRIBUTE_PRINTF(2,3);
  void indentBegin();
  void indentEnd(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
    ATTRIBUTE_PRINTF(2,3);
  void indentEnd();
  void printRaw(const char *msg) { print(msg, false);}
  /**
   * Pre-formatted outputs.
   */
  void printSection(const char *name, bool newline = true);
  void printSeparator();
  void namespaceBegin();
  void namespaceEnd();
  bool ensureInNamespace();
  bool ensureOutOfNamespace();
  void ifdefBegin(bool ifdef, ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
    ATTRIBUTE_PRINTF(3,4);
  void ifdefEnd(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
    ATTRIBUTE_PRINTF(2,3);
  void printDocComment(const std::string comment);
  const char *getGlobals(AnalysisResultPtr ar);
  static std::string EscapeLabel(const std::string &name, bool *binary = nullptr);

  /**
   * Make sure PHP variables, functions and typenames are unique and
   * different from C's built-in keywords and HPHP's.
   */
  void resetIdCount(const std::string &key);
  int createNewId(const std::string &key);
  int createNewId(ConstructPtr ar);
  int createNewLocalId(ConstructPtr ar);
  static std::string GetNewLambda(); // for create_function()

  /**
   * Contexts allow one construct generates more than one kind of source.
   * For example, a ClassStatement generates different source code, depending
   * on whether we are generating a header file or an implementation file.
   */
  void setContext(Context context) { m_context = context;}
  Context getContext() const { return m_context;}

  /**
    * Remember comment nested level to avoid double comments.
   */
  bool inComments() const;
  void startComments();
  void endComments();

  /**
   * Helpers for break/continue scopes.
   */
  void pushBreakScope(int labelId, bool loopCounter = true);
  void popBreakScope();
  const std::vector<int> &getBreakScopes() const { return m_breakScopes;}
  void addLabelId(const char *name, int labelId);
  bool findLabelId(const char *name, int labelId);

  /**
   * Get current line number of primary stream.
   */
  int getLineNo(Stream stream) const;

  int getPHPLineNo() { return m_phpLineNo; }
  void setPHPLineNo(int ln) { m_phpLineNo = ln; }

  bool translatePredefined() { return m_translatePredefined; }
  void translatePredefined(bool flag) { m_translatePredefined = flag; }

  int checkLiteralString(const std::string &str, int &index,
                         AnalysisResultPtr ar, BlockScopePtr bs,
                         bool scalarVariant = false);
  std::string printNamedString(const std::string &str,
                               const std::string &escaped,
                               AnalysisResultPtr ar, BlockScopeRawPtr bs,
                               bool print);
  std::string printString(const std::string &str, AnalysisResultPtr ar,
                          BlockScopeRawPtr check, bool stringWrapper = true);
  std::string printString(const std::string &str, AnalysisResultPtr ar,
                          ConstructPtr check, bool stringWrapper = true);
  int getCurrentIndentation() const { return m_indentation[m_curStream];}

  bool inExpression() { return m_inExpression[m_curStream]; }
  void setInExpression(bool in) { m_inExpression[m_curStream] = in; }

  void pushCallInfo(int cit);
  void popCallInfo();
  int callInfoTop();

  LoopStatementPtr getLoopStatement() const { return m_loopStatement; }
  void setLoopStatement(LoopStatementPtr loop) {
    m_loopStatement = loop;
  }

  void setFileOrClassHeader(bool value) { m_inFileOrClassHeader = value; }
  bool isFileOrClassHeader() { return m_inFileOrClassHeader; }

  void setInitListFirstElem() { m_initListFirstElem = true; }
  bool hasInitListFirstElem() { return m_initListFirstElem; }
  void clearInitListFirstElem() { m_initListFirstElem = false; }

  bool insertDeclaredClosure(const FunctionScope *f) {
    return m_declaredClosures.insert(f).second;
  }
  void setLiteralScope(FileScopeRawPtr fs) {
    m_literalScope = fs;
  }
  FileScopeRawPtr getLiteralScope() const {
    return m_literalScope;
  }

private:
  std::string m_filename;
  Stream m_curStream;
  std::ostream *m_streams[StreamCount];
  std::ostream *m_out;
  Output m_output;
  bool m_verbose;

  int m_indentation[StreamCount];
  bool m_indentPending[StreamCount];
  int m_lineNo[StreamCount];
  int m_inComments[StreamCount];
  bool m_wrappedExpression[StreamCount];
  std::string m_referenceTemps[StreamCount];
  bool m_referenceTempsUsed[StreamCount];
  bool m_inExpression[StreamCount];
  bool m_inFileOrClassHeader;
  bool m_inNamespace;
  int m_localId[StreamCount];

  static int s_idLambda;
  std::map<std::string, int> m_idCounters;
  Context m_context;
  std::vector<int> m_breakScopes;
  std::set<int> m_breakLabelIds; // break labels referenced
  std::set<int> m_contLabelIds;  // continue labels referenced
  std::deque<int> m_callInfos;
  LoopStatementPtr m_loopStatement;
  StringToClassScopePtrVecMap m_classes;
  std::set<const FunctionScope*> m_declaredClosures;
  FileScopeRawPtr m_literalScope;

  int m_itemIndex;

  int m_phpLineNo;

  bool m_translatePredefined; // translate predefined constants in PHP output
  bool m_scalarVariant;
  bool m_initListFirstElem;

  public: void print(const char *msg, bool indent = true);

 private:
  void print(ATTRIBUTE_PRINTF_STRING const char *fmt, va_list ap)
    ATTRIBUTE_PRINTF(2,0);
  void printSubstring(const char *start, int length);
  void printIndent();
  std::string getFormattedName(const std::string &file);
};

#define cg_printf cg.printf
#define m_cg_printf m_cg.printf
#define cg_print cg.print
#define m_cg_print m_cg.print
#define cg_indentBegin cg.indentBegin
#define m_cg_indentBegin m_cg.indentBegin
#define cg_indentEnd cg.indentEnd
#define m_cg_indentEnd cg.indentEnd
#define cg_printInclude cg.printInclude
#define cg_printString cg.printString

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CODE_GENERATOR_H_

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

#ifndef __CODE_GENERATOR_H__
#define __CODE_GENERATOR_H__

#include <compiler/hphp.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(Statement);

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
    CppStaticInitializer,       // we are genearting static initializer
    CppLazyStaticInitializer,   // lazy initializer for dynamic statics
    CppClassConstantsDecl,
    CppClassConstantsImpl,
    CppConstantsDecl,           // we are generating constant declarations
    CppStaticMethodWrapper,     // Only used to force parameters to use C*Ref
    CppParameterDefaultValueDecl,
    CppParameterDefaultValueImpl,
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
  /**
   * Hash strings to numbers so we can build a switch statement.
   */
  typedef std::map<int, std::vector<const char *> > MapIntToStringVec;
  static void BuildJumpTable(const std::vector<const char *> &strings,
                             MapIntToStringVec &out, int tableSize,
                             bool caseInsensitive);

public:
  CodeGenerator(std::ostream *primary, Output output = PickledPHP,
                std::string *filename = NULL);

  /**
   * ...if it was passed in from constructor.
   */
  std::string getFileName() const { return m_filename;}

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
  void printf(const char *fmt, ...);
  void indentBegin(const char *fmt, ...);
  void indentEnd(const char *fmt, ...);

  /**
   * Pre-formatted outputs.
   */
  void printSection(const char *name, bool newline = true);
  void printSeparator();
  void namespaceBegin();
  void namespaceEnd();
  void headerBegin(const std::string &file);
  void headerEnd(const std::string &file);
  void printInclude(const std::string &file);
  void printDeclareGlobals();
  void printStartOfJumpTable(int tableSize);
  const char *getGlobals(AnalysisResultPtr ar);

  /**
   * Make sure PHP variables, functions and typenames are unique and
   * different from C's built-in keywords and HPHP's.
   */
  void resetIdCount(const std::string &key);
  int createNewId(const std::string &key);
  int createNewId(AnalysisResultPtr ar);
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
   * Helpers for keeping track of index in an ExpressionList.
   */
  void setItemIndex(int index) { m_itemIndex = index;}
  int getItemIndex() { return m_itemIndex;}

  /**
   * Get current line number of primary stream.
   */
  int getLineNo(Stream stream) const;

  int getPHPLineNo() { return m_phpLineNo; }
  void setPHPLineNo(int ln) { m_phpLineNo = ln; }

  bool translatePredefined() { return m_translatePredefined; }
  void translatePredefined(bool flag) { m_translatePredefined = flag; }

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

  static int s_idLambda;
  std::map<std::string, int> m_idCounters;
  Context m_context;
  std::vector<int> m_breakScopes;
  std::set<int> m_breakLabelIds; // break labels referenced
  std::set<int> m_contLabelIds;  // continue labels referenced
  int m_itemIndex;

  int m_phpLineNo;

  bool m_translatePredefined; // translate predefined constants in PHP output

  void print(const char *fmt, va_list ap);
  void print(const std::string &msg);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __CODE_GENERATOR_H__

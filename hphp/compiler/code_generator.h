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

struct CodeGenerator {
  enum Output {
    InvalidOutput,

    PickledPHP, // stripped comments, etc. but 1 to 1 file to file
    InlinedPHP, // all includes are inlined
    TrimmedPHP, // unreferenced functions and classes are removed
    TextHHBC,   // HHBC dump in human-readable format
  };

  enum Stream {
    NullStream = -1,    // suppress output
    PrimaryStream,      // main output

    StreamCount
  };

  enum Context {
    NoContext,

    PhpDeclaration,
    PhpImplementation,
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

  /**
   * Stream functions.
   */
  void useStream(Stream stream);
  bool usingStream(Stream stream);
  std::ostream *getStream() const { return m_out;}
  void setStream(Stream stream, std::ostream *out);

  /**
   * Output strings.
   */
  void printf(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
    ATTRIBUTE_PRINTF(2,3);
  void indentBegin(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
    ATTRIBUTE_PRINTF(2,3);
  void indentEnd(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
    ATTRIBUTE_PRINTF(2,3);
  void indentEnd();
  void printRaw(const char *msg) { print(msg, false);}
  /**
   * Pre-formatted outputs.
   */
  void printSeparator();
  void namespaceBegin();
  void namespaceEnd();

  /**
   * Make sure PHP variables, functions and typenames are unique and
   * different from C's built-in keywords and HPHP's.
   */
  int createNewId(const std::string &key);
  static std::string GetNewLambda(); // for create_function()

  /**
   * Contexts allow one construct generates more than one kind of source.
   * For example, a ClassStatement generates different source code, depending
   * on whether we are generating a header file or an implementation file.
   */
  void setContext(Context context) { m_context = context;}
  Context getContext() const { return m_context;}

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
  bool m_wrappedExpression[StreamCount];
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
};

#define cg_printf cg.printf
#define cg_print cg.print
#define cg_indentBegin cg.indentBegin
#define cg_indentEnd cg.indentEnd
#define cg_printInclude cg.printInclude
#define cg_printString cg.printString

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CODE_GENERATOR_H_

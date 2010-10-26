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

#include <stdarg.h>

#include <compiler/code_generator.h>
#include <compiler/statement/statement_list.h>
#include <compiler/option.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/variable_table.h>
#include <util/util.h>
#include <util/hash.h>
#include <boost/format.hpp>

using namespace HPHP;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
// statics

void CodeGenerator::BuildJumpTable(const std::vector<const char *> &strings,
                                   MapIntToStringVec &out, int tableSize,
                                   bool caseInsensitive) {
  ASSERT(!strings.empty());
  ASSERT(out.empty());
  ASSERT(tableSize > 0);

  for (unsigned int i = 0; i < strings.size(); i++) {
    const char *s = strings[i];
    int hash = (caseInsensitive ? hash_string_i(s) : hash_string(s)) %
               tableSize;
    out[hash].push_back(s);
  }
}

const char *CodeGenerator::STARTER_MARKER =
  "namespace hphp_impl_starter {}";
const char *CodeGenerator::SPLITTER_MARKER =
  "namespace hphp_impl_splitter {}";

///////////////////////////////////////////////////////////////////////////////

CodeGenerator::CodeGenerator(std::ostream *primary,
                             Output output /* = PickledPHP */,
                             std::string *filename /* = NULL */)
    : m_out(NULL), m_output(output), m_context(NoContext),
      m_insideScalarArray(false), m_itemIndex(-1) {
  for (int i = 0; i < StreamCount; i++) {
    m_streams[i] = NULL;
    m_indentation[i] = 0;
    m_indentPending[i] = false;
    m_lineNo[i] = 1;
    m_inComments[i] = 0;
    m_wrappedExpression[i] = false;
    m_inExpression[i] = false;
  }
  setStream(PrimaryStream, primary);
  useStream(PrimaryStream);

  if (filename) m_filename = *filename;
  m_translatePredefined = false;
}

void CodeGenerator::useStream(Stream stream) {
  ASSERT(stream >= NullStream && stream < StreamCount);
  m_curStream = stream;
  if (stream == NullStream) {
    m_out = NULL;
  } else {
    m_out = m_streams[stream];
  }
}

bool CodeGenerator::usingStream(Stream stream) {
  ASSERT(stream >= 0 && stream < StreamCount);
  return m_out == m_streams[stream];
}

std::ostream *CodeGenerator::getStream(Stream stream) const {
  ASSERT(stream >= 0 && stream < StreamCount);
  return m_streams[stream];
}

void CodeGenerator::setStream(Stream stream, std::ostream *out) {
  ASSERT(out);
  ASSERT(stream >= 0 && stream < StreamCount);
  m_streams[stream] = out;
}

int CodeGenerator::getLineNo(Stream stream) const {
  ASSERT(stream >= 0 && stream < StreamCount);
  return m_lineNo[stream];
}

///////////////////////////////////////////////////////////////////////////////

void CodeGenerator::printf(const char *fmt, ...) {
  va_list ap; va_start(ap, fmt); print(fmt, ap); va_end(ap);
}

void CodeGenerator::indentBegin(const char *fmt, ...) {
  va_list ap; va_start(ap, fmt); print(fmt, ap); va_end(ap);
  m_indentation[m_curStream]++;
}

void CodeGenerator::indentEnd(const char *fmt, ...) {
  ASSERT(m_indentation[m_curStream]);
  m_indentation[m_curStream]--;
  va_list ap; va_start(ap, fmt); print(fmt, ap); va_end(ap);
}

bool CodeGenerator::wrapExpressionBegin() {
  if (!m_wrappedExpression[m_curStream]) {
    m_wrappedExpression[m_curStream] = true;
    indentBegin("{\n");
    return true;
  }
  return false;
}

bool CodeGenerator::wrapExpressionEnd() {
  if (m_wrappedExpression[m_curStream]) {
    m_wrappedExpression[m_curStream] = false;
    indentEnd("}\n");
    return true;
  }
  return false;
}

bool CodeGenerator::inComments() const {
  return m_inComments[m_curStream] > 0;
}

void CodeGenerator::startComments() {
  m_inComments[m_curStream]++;
  printf(" /*");
}

void CodeGenerator::endComments() {
  ASSERT(m_inComments[m_curStream] > 0);
  m_inComments[m_curStream]--;
  printf(" */");
}

void CodeGenerator::printSection(const char *name, bool newline /* = true */) {
  if (newline) printf("\n");
  printf("// %s\n", name);
}

void CodeGenerator::printSeparator() {
  printf("///////////////////////////////////////"
         "////////////////////////////////////////\n");
}

void CodeGenerator::namespaceBegin() {
  printf("\n");
  printf("namespace HPHP {\n");
  printSeparator();
  printf("\n");
}

void CodeGenerator::namespaceEnd() {
  printf("\n");
  printSeparator();
  printf("}\n");
}

std::string CodeGenerator::getFormattedName(const std::string &file) {
  string formatted = file;
  Util::replaceAll(formatted, ".", "_");
  Util::replaceAll(formatted, "/", "_");
  Util::replaceAll(formatted, "-", "_");
  Util::replaceAll(formatted, "$", "_");

  int hash = hash_string(file.data(), file.size());
  formatted += boost::str(boost::format("%08x") % hash);
  return formatted;
}

void CodeGenerator::headerBegin(const std::string &file) {
  string formatted = getFormattedName(file);
  printf("\n");
  printf("#ifndef __GENERATED_%s__\n", formatted.c_str());
  printf("#define __GENERATED_%s__\n", formatted.c_str());
  printf("\n");
}

void CodeGenerator::headerEnd(const std::string &file) {
  string formatted = getFormattedName(file);
  printf("\n");
  printf("#endif // __GENERATED_%s__\n", formatted.c_str());
}

void CodeGenerator::ifdefBegin(bool ifdef, const char *fmt, ...) {
  printf(ifdef ? "#ifdef " : "#ifndef ");
  va_list ap; va_start(ap, fmt); print(fmt, ap); va_end(ap);
  printf("\n");
}

void CodeGenerator::ifdefEnd(const char *fmt, ...) {
  printf("#endif // ");
  va_list ap; va_start(ap, fmt); print(fmt, ap); va_end(ap);
  printf("\n");
}

void CodeGenerator::printInclude(const std::string &file) {
  ASSERT(!file.empty());

  string formatted = file;
  if (file[0] != '"' && file[0] != '<') {
    if (file.substr(file.length() - 2) != ".h") {
      formatted += ".h";
    }
    formatted = string("<") + formatted + '>';
  }
  printf("#include %s\n", formatted.c_str());
}

void CodeGenerator::printBasicIncludes() {
  if (Option::GenerateCPPMain) {
    printInclude("<runtime/base/hphp.h>");
    printInclude(string(Option::SystemFilePrefix) +
                 "literal_strings_remap.h");
    printInclude(string(Option::SystemFilePrefix) +
                 "scalar_arrays_remap.h");
    printInclude(string(Option::SystemFilePrefix) + "global_variables.h");
    if (Option::GenConcat || Option::GenArrayCreate) {
      printInclude(string(Option::SystemFilePrefix) + "cpputil.h");
    }
  } else if (getOutput() == CodeGenerator::SystemCPP) {
    printInclude("<runtime/base/hphp_system.h>");
    printInclude(string(Option::SystemFilePrefix) +
                 "literal_strings_remap.h");
    printInclude(string(Option::SystemFilePrefix) +
                 "scalar_arrays_remap.h");
  }
}

void CodeGenerator::printDeclareGlobals() {
  if (getOutput() == SystemCPP) {
    printf("DECLARE_SYSTEM_GLOBALS(g);\n");
  } else {
    printf("DECLARE_GLOBAL_VARIABLES(g);\n");
  }
}

void CodeGenerator::printStartOfJumpTable(int tableSize) {
  if (Util::isPowerOfTwo(tableSize)) {
    indentBegin("switch (hash & %d) {\n", tableSize-1);
  } else {
    indentBegin("switch (hash %% %d) {\n", tableSize);
  }
}

void CodeGenerator::printDocComment(const std::string comment) {
  if (comment.empty()) return;
  string escaped;
  escaped.reserve(comment.size() + 10);
  for (unsigned int i = 0; i < comment.size(); i++) {
    char ch = comment[i];
    escaped += ch;
    if (ch == '/' && i > 1 && comment[i+1] == '*') {
      escaped += '\\'; // splitting illegal /* into /\*
    }
  }
  print(escaped, false);
  printf("\n");
}

void CodeGenerator::printImplStarter() {
  printf("%s\n", STARTER_MARKER);
}

void CodeGenerator::printImplSplitter() {
  printf("%s\n", SPLITTER_MARKER);
}

const char *CodeGenerator::getGlobals(AnalysisResultPtr ar) {
  if (m_context == CppParameterDefaultValueDecl ||
      m_context == CppParameterDefaultValueImpl) {
    return (m_output == CodeGenerator::SystemCPP) ?
           "get_system_globals()" : "get_global_variables()";
  }
  if (m_output == CodeGenerator::SystemCPP) return "get_system_globals()";
  return "g";
}

std::string CodeGenerator::formatLabel(const std::string &name) {
  string ret;
  ret.reserve(name.size());
  for (size_t i = 0; i < name.size(); i++) {
    unsigned char ch = name[i];
    if ((ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z') ||
        (ch >= '0' && ch <= '9') || ch == '_') {
      ret += ch;
    } else {
      char buf[10];
      snprintf(buf, sizeof(buf), "%s%02X", Option::LabelEscape.c_str(),
               (int)ch);
      ret += buf;
    }
  }
  return ret;
}

std::string CodeGenerator::escapeLabel(const std::string &name,
                                       bool *binary /* = NULL */) {
  return Util::escapeStringForCPP(name, binary);
}

///////////////////////////////////////////////////////////////////////////////
// helpers

void CodeGenerator::print(const char *fmt, va_list ap) {
  string msg;
  bool done = false;
  for (int len = 1024; !done; len <<= 1) {
    va_list v;
    va_copy(v, ap);

    char *buf = (char*)malloc(len);
    if (vsnprintf(buf, len, fmt, v) < len) {
      msg = buf;
      done = true;
    }
    free(buf);

    va_end(v);
  }
  if (m_out) {
    print(msg);
  }
}

void CodeGenerator::print(const std::string &msg, bool indent /* = true */) {
  // empty line doesn't need indentation
  if (msg.size() == 1 && msg[0] == '\n') {
    *m_out << '\n';
    m_lineNo[m_curStream]++;
    m_indentPending[m_curStream] = true;
    return;
  }

  if (m_indentPending[m_curStream]) {
    m_indentPending[m_curStream] = false;
    for (int i = 0; i < m_indentation[m_curStream]; i++) {
      *m_out << Option::Tab;
    }
  }
  for (unsigned int i = 0; i < msg.length(); i++) {
    unsigned char ch = msg[i];
    *m_out << ch;
    if (ch == '\n' && indent) {
      m_lineNo[m_curStream]++;
      if (m_indentPending[m_curStream]) {
        for (int i = 0; i < m_indentation[m_curStream]; i++) {
          *m_out << Option::Tab;
        }
      }
      m_indentPending[m_curStream] = true;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

int CodeGenerator::s_idLambda = 0;
string CodeGenerator::GetNewLambda() {
  return Option::LambdaPrefix + "lambda_" +
    boost::lexical_cast<string>(++s_idLambda);
}

void CodeGenerator::resetIdCount(const std::string &key) {
  m_idCounters[key] = 0;
}

int CodeGenerator::createNewId(const std::string &key) {
  return ++m_idCounters[key];
}

int CodeGenerator::createNewId(ConstructPtr cs) {
  FileScopePtr fs = cs->getFileScope();
  if (fs) {
    return createNewId(fs->getName());
  }
  return createNewId("");
}

int CodeGenerator::createNewLocalId(ConstructPtr ar) {
  FunctionScopePtr func = ar->getFunctionScope();
  if (func) {
    return func->nextInlineIndex();
  }
  FileScopePtr fs = ar->getFileScope();
  if (fs) {
    return createNewId(fs->getName());
  }
  return createNewId("");
}

void CodeGenerator::pushBreakScope(int labelId,
                                   bool loopCounter /* = true */) {
  m_breakScopes.push_back(labelId);
  if (loopCounter) {
    printf("LOOP_COUNTER(%d);\n", labelId & ~BreakScopeBitMask);
  }
}

void CodeGenerator::popBreakScope() {
  m_breakScopes.pop_back();
  if (m_breakScopes.size() == 0) {
    m_breakLabelIds.clear();
    m_contLabelIds.clear();
  }
}

void CodeGenerator::pushCallInfo(int cit) {
  m_callInfos.push_back(cit);
}
void CodeGenerator::popCallInfo() {
  m_callInfos.pop_back();
}
int CodeGenerator::callInfoTop() {
  if (m_callInfos.empty()) return -1;
  return m_callInfos.back();
}

bool CodeGenerator::getInsideScalarArray() {
  return m_insideScalarArray;
}

void CodeGenerator::setInsideScalarArray(bool flag) {
  m_insideScalarArray = flag;
}

void CodeGenerator::addLabelId(const char *name, int labelId) {
  if (!strcmp(name, "break")) {
    m_breakLabelIds.insert(labelId);
  } else if (!strcmp(name, "continue")) {
    m_contLabelIds.insert(labelId);
  } else {
    ASSERT(false);
  }
}

bool CodeGenerator::findLabelId(const char *name, int labelId) {
  if (!strcmp(name, "break")) {
    return m_breakLabelIds.find(labelId) != m_breakLabelIds.end();
  } else if (!strcmp(name, "continue")) {
    return m_contLabelIds.find(labelId) != m_contLabelIds.end();
  } else {
    ASSERT(false);
  }
  return false;
}

int CodeGenerator::checkLiteralString(const std::string &str, int &index,
                                      AnalysisResultPtr ar, BlockScopePtr bs) {
  if (getContext() == CodeGenerator::CppConstantsDecl ||
      getContext() == CodeGenerator::CppClassConstantsImpl) {
    assert(false);
  }
  int stringId = ar->getLiteralStringId(str, index);
  if (bs && bs != ar) {
    FileScopePtr fs = bs->getContainingFile();
    if (fs) {
      fs->addUsedLiteralString(str);
      if (m_context == CppParameterDefaultValueDecl ||
          m_context == CppStaticMethodWrapper) {
        fs->addUsedLiteralStringHeader(str);
      }
    }
  }
  return stringId;
}

void CodeGenerator::printString(const std::string &str, AnalysisResultPtr ar,
                                BlockScopeRawPtr bs,
                                bool stringWrapper /* = true */) {
  int index = -1;
  bool isBinary = false;
  string escaped = escapeLabel(str, &isBinary);
  if (bs) {
    int stringId = checkLiteralString(str, index, ar, bs);
    assert(index >= 0);
    string lisnam = ar->getLiteralStringName(stringId, index);
    printf("NAMSTR(%s, \"%s\")", lisnam.c_str(), escaped.c_str());
  } else if (isBinary) {
    if (stringWrapper) {
      printf("String(\"%s\", %d, AttachLiteral)",
             escaped.c_str(), str.length());
    } else {
      printf("\"%s\", %d", escaped.c_str(), str.length());
    }
  } else {
    printf("\"%s\"", escaped.c_str());
  }
}

void CodeGenerator::printString(const std::string &str, AnalysisResultPtr ar,
                                ConstructPtr cs,
                                bool stringWrapper /* = true */) {
  printString(str, ar, (BlockScopePtr)cs->getScope(), stringWrapper);
}

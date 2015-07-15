/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/code_generator.h"
#include "hphp/compiler/code_model_enums.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/util/text-util.h"
#include "hphp/util/hash.h"
#include <folly/Conv.h>
#include <boost/format.hpp>
#include <boost/scoped_array.hpp>
#include <algorithm>
#include <vector>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

CodeGenerator::CodeGenerator(std::ostream *primary,
                             Output output /* = PickledPHP */,
                             const std::string *filename /* = NULL */)
    : m_out(nullptr), m_output(output),
      m_context(NoContext), m_itemIndex(-1) {
  for (int i = 0; i < StreamCount; i++) {
    m_streams[i] = nullptr;
    m_indentation[i] = 0;
    m_indentPending[i] = true;
    m_lineNo[i] = 1;
    m_inComments[i] = 0;
    m_wrappedExpression[i] = false;
    m_inExpression[i] = false;
  }
  setStream(PrimaryStream, primary);
  useStream(PrimaryStream);

  if (filename) m_filename = *filename;
  m_translatePredefined = false;
  m_scalarVariant = false;
  m_initListFirstElem = false;
  m_inFileOrClassHeader = false;
  m_inNamespace = false;
}

void CodeGenerator::useStream(Stream stream) {
  assert(stream >= NullStream && stream < StreamCount);
  m_curStream = stream;
  if (stream == NullStream) {
    m_out = nullptr;
  } else {
    m_out = m_streams[stream];
  }
}

bool CodeGenerator::usingStream(Stream stream) {
  assert(stream >= 0 && stream < StreamCount);
  return m_out == m_streams[stream];
}

std::ostream *CodeGenerator::getStream(Stream stream) const {
  assert(stream >= 0 && stream < StreamCount);
  return m_streams[stream];
}

void CodeGenerator::setStream(Stream stream, std::ostream *out) {
  assert(out);
  assert(stream >= 0 && stream < StreamCount);
  m_streams[stream] = out;
}

int CodeGenerator::getLineNo(Stream stream) const {
  assert(stream >= 0 && stream < StreamCount);
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

void CodeGenerator::indentBegin() {
  m_indentation[m_curStream]++;
}

void CodeGenerator::indentEnd(const char *fmt, ...) {
  assert(m_indentation[m_curStream]);
  m_indentation[m_curStream]--;
  va_list ap; va_start(ap, fmt); print(fmt, ap); va_end(ap);
}

void CodeGenerator::indentEnd() {
  assert(m_indentation[m_curStream]);
  m_indentation[m_curStream]--;
}

bool CodeGenerator::inComments() const {
  return m_inComments[m_curStream] > 0;
}

void CodeGenerator::startComments() {
  m_inComments[m_curStream]++;
  printf(" /*");
}

void CodeGenerator::endComments() {
  assert(m_inComments[m_curStream] > 0);
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
  always_assert(!m_inNamespace);
  m_inNamespace = true;
  printf("\n");
  printf("namespace HPHP {\n");
  printSeparator();
  printf("\n");
}

void CodeGenerator::namespaceEnd() {
  always_assert(m_inNamespace);
  m_inNamespace = false;
  printf("\n");
  printSeparator();
  printf("}\n");
}

std::string CodeGenerator::getFormattedName(const std::string &file) {
  char *fn = strdup(file.c_str());
  int len = strlen(fn);
  always_assert(len == (int)file.size());
  for (int i = 0; i < len; i++) {
    if (!isalnum(fn[i])) fn[i] = '_';
  }
  string formatted = fn;
  free(fn);
  int hash = hash_string_unsafe(file.data(), file.size());
  formatted += boost::str(boost::format("%08x") % hash);
  return formatted;
}

bool CodeGenerator::ensureInNamespace() {
  if (m_inNamespace) return false;
  namespaceBegin();
  return true;
}

bool CodeGenerator::ensureOutOfNamespace() {
  if (!m_inNamespace) return false;
  namespaceEnd();
  return true;
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
  print(escaped.c_str(), false);
  printf("\n");
}

std::string CodeGenerator::EscapeLabel(const std::string &name,
                                       bool *binary /* = NULL */) {
  return escapeStringForCPP(name, binary);
}

///////////////////////////////////////////////////////////////////////////////
// helpers

void CodeGenerator::print(const char *fmt, va_list ap) {
  if (!m_out) return;
  boost::scoped_array<char> buf;
  bool done = false;
  for (int len = 1024; !done; len <<= 1) {
    va_list v;
    va_copy(v, ap);

    buf.reset(new char[len]);
    if (vsnprintf(buf.get(), len, fmt, v) < len) done = true;

    va_end(v);
  }
  print(buf.get());
}

void CodeGenerator::print(const char *msg, bool indent /* = true */) {
  const char *start = msg;
  int length = 1;
  for (const char *iter = msg; ; ++iter, ++length) {
    if (*iter == '\n') {
      if (indent) {
        // Only indent if it is pending and if it is not an empty line
        if (m_indentPending[m_curStream] && length > 1) printIndent();

        // Printing substrings requires an additional copy operation,
        // so do it only if necessary
        if (iter[1] != '\0') {
          printSubstring(start, length);
        } else {
          *m_out << start;
        }
        start = iter + 1;
        length = 0;
      }
      m_lineNo[m_curStream]++;
      m_indentPending[m_curStream] = true;
    } else if (*iter == '\0') {
      // Perform print only in case what's left is not an empty string
      if (length > 1) {
        if (indent && m_indentPending[m_curStream]) {
          printIndent();
          m_indentPending[m_curStream] = false;
        }
        *m_out << start;
      }
      break;
    }
  }
}

void CodeGenerator::printSubstring(const char *start, int length) {
  const int BUF_LEN = 0x100;
  char buf[BUF_LEN];
  while (length > 0) {
    int curLength = std::min(length, BUF_LEN - 1);
    memcpy(buf, start, curLength);
    buf[curLength] = '\0';
    *m_out << buf;
    length -= curLength;
    start += curLength;
  }
}

void CodeGenerator::printIndent() {
  for (int i = 0; i < m_indentation[m_curStream]; i++) {
    *m_out << Option::Tab;
  }
}

///////////////////////////////////////////////////////////////////////////////

int CodeGenerator::s_idLambda = 0;
string CodeGenerator::GetNewLambda() {
  return Option::LambdaPrefix + "lambda_" + folly::to<string>(++s_idLambda);
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
  if (m_wrappedExpression[m_curStream]) {
    return m_localId[m_curStream]++;
  }
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
    printf("LOOP_COUNTER(%d);\n", int(labelId & ~BreakScopeBitMask));
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

void CodeGenerator::addLabelId(const char *name, int labelId) {
  if (!strcmp(name, "break")) {
    m_breakLabelIds.insert(labelId);
  } else if (!strcmp(name, "continue")) {
    m_contLabelIds.insert(labelId);
  } else {
    assert(false);
  }
}

bool CodeGenerator::findLabelId(const char *name, int labelId) {
  if (!strcmp(name, "break")) {
    return m_breakLabelIds.find(labelId) != m_breakLabelIds.end();
  } else if (!strcmp(name, "continue")) {
    return m_contLabelIds.find(labelId) != m_contLabelIds.end();
  } else {
    assert(false);
  }
  return false;
}

void CodeGenerator::printObjectHeader(const std::string className,
                                      int numProperties) {
  std::string prefixedClassName;
  prefixedClassName.append(m_astPrefix);
  prefixedClassName.append(className);
  m_astClassNames.push_back(prefixedClassName);
  printf("O:%d:\"%s\":%d:{",
    (int)prefixedClassName.length(), prefixedClassName.c_str(), numProperties);
}

void CodeGenerator::printObjectFooter() {
  printf("}");
  m_astClassNames.pop_back();
}

void CodeGenerator::printPropertyHeader(const std::string propertyName) {
  auto prefixedClassName = m_astClassNames.back();
  auto len = 2+prefixedClassName.length()+propertyName.length();
  printf("s:%d:\"", (int)len);
  *m_out << (char)0;
  printf("%s", prefixedClassName.c_str());
  *m_out << (char)0;
  printf("%s\";", propertyName.c_str());
}

void CodeGenerator::printNull() {
  printf("N;");
}

void CodeGenerator::printBool(bool value) {
  printf("b:%d;", value ? 1 : 0);
}

void CodeGenerator::printValue(double v) {
  *m_out << "d:";
  if (std::isnan(v)) {
    *m_out << "NAN";
  } else if (std::isinf(v)) {
    if (v < 0) *m_out << '-';
    *m_out << "INF";
  } else {
    char *buf;
    if (v == 0.0) v = 0.0; // so to avoid "-0" output
    vspprintf(&buf, 0, "%.*H", 14, v);
    m_out->write(buf, strlen(buf));
    free(buf);
  }
  *m_out << ';';
}

void CodeGenerator::printValue(int32_t value) {
  printf("i:%d;", value);
}

void CodeGenerator::printValue(int64_t value) {
  printf("i:%" PRId64 ";", value);
}

void CodeGenerator::printValue(std::string value) {
  printf("s:%d:\"", (int)value.length());
  getStream()->write(value.c_str(), value.length());
  printf("\";");
}

void CodeGenerator::printModifierVector(std::string value) {
  printf("V:9:\"HH\\Vector\":1:{");
  printObjectHeader("Modifier", 1);
  printPropertyHeader("name");
  printValue(value);
  printObjectFooter();
  printf("}");
}

void CodeGenerator::printTypeExpression(std::string value) {
  printObjectHeader("TypeExpression", 1);
  printPropertyHeader("name");
  printValue(value);
  printObjectFooter();
}

void CodeGenerator::printTypeExpression(ExpressionPtr expression) {
  printObjectHeader("TypeExpression", 2);
  printPropertyHeader("name");
  expression->outputCodeModel(*this);
  printPropertyHeader("sourceLocation");
  printLocation(expression);
  printObjectFooter();
}

void CodeGenerator::printExpression(ExpressionPtr expression, bool isRef) {
  if (isRef) {
    printObjectHeader("UnaryOpExpression", 3);
    printPropertyHeader("expression");
    expression->outputCodeModel(*this);
    printPropertyHeader("operation");
    printValue(PHP_REFERENCE_OP);
    printPropertyHeader("sourceLocation");
    printLocation(expression);
    printObjectFooter();
  } else {
    expression->outputCodeModel(*this);
  }
}

void CodeGenerator::printExpressionVector(ExpressionListPtr el) {
  auto count = el == nullptr ? 0 : el->getCount();
  printf("V:9:\"HH\\Vector\":%d:{", count);
  if (count > 0) {
    el->outputCodeModel(*this);
  }
  printf("}");
}

void CodeGenerator::printExpressionVector(ExpressionPtr e) {
  if (e->is(Expression::KindOfExpressionList)) {
    auto sl = static_pointer_cast<ExpressionList>(e);
    printExpressionVector(sl);
  } else {
    printf("V:9:\"HH\\Vector\":1:{");
    e->outputCodeModel(*this);
    printf("}");
  }
}

void CodeGenerator::printTypeExpressionVector(ExpressionListPtr el) {
  auto count = el == nullptr ? 0 : el->getCount();
  printf("V:9:\"HH\\Vector\":%d:{", count);
  for (int i = 0; i < count; i++) {
    auto te = (*el)[i];
    printTypeExpression(te);
  }
  printf("}");
}

void CodeGenerator::printAsBlock(StatementPtr s, bool isEnclosed) {
  if (s != nullptr && s->is(Statement::KindOfBlockStatement)) {
    s->outputCodeModel(*this);
  } else {
    auto numProps = s == nullptr ? 0 : 2;
    if (isEnclosed) numProps++;
    printObjectHeader("BlockStatement", numProps);
    if (s != nullptr) {
      printPropertyHeader("statements");
      printStatementVector(s);
      printPropertyHeader("sourceLocation");
      printLocation(s);
    }
    if (isEnclosed) {
      printPropertyHeader("isEnclosed");
      printBool(true);
    }
    printObjectFooter();
  }
}

void CodeGenerator::printStatementVector(StatementListPtr sl) {
  printf("V:9:\"HH\\Vector\":%d:{", sl->getCount());
  if (sl->getCount() > 0) {
    sl->outputCodeModel(*this);
  }
  printf("}");
}

void CodeGenerator::printStatementVector(StatementPtr s) {
  if (s == nullptr) {
    printf("V:9:\"HH\\Vector\":0:{}");
  } else if (s->is(Statement::KindOfStatementList)) {
    auto sl = static_pointer_cast<StatementList>(s);
    printStatementVector(sl);
  } else {
    printf("V:9:\"HH\\Vector\":1:{");
    s->outputCodeModel(*this);
    printf("}");
  }
}

void CodeGenerator::printLocation(const Construct* what) {
  if (what == nullptr) return;
  auto r = what->getRange();
  printObjectHeader("SourceLocation", 4);
  printPropertyHeader("startLine");
  printValue(r.line0);
  printPropertyHeader("endLine");
  printValue(r.line1);
  printPropertyHeader("startColumn");
  printValue(r.char0);
  printPropertyHeader("endColumn");
  printValue(r.char1);
  printObjectFooter();
}

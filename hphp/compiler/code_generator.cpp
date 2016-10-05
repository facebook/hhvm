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

#include "hphp/compiler/code_generator.h"
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
      m_context(NoContext) {
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

void CodeGenerator::setStream(Stream stream, std::ostream *out) {
  assert(out);
  assert(stream >= 0 && stream < StreamCount);
  m_streams[stream] = out;
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
  assert(m_indentation[m_curStream]);
  m_indentation[m_curStream]--;
  va_list ap; va_start(ap, fmt); print(fmt, ap); va_end(ap);
}

void CodeGenerator::indentEnd() {
  assert(m_indentation[m_curStream]);
  m_indentation[m_curStream]--;
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
std::string CodeGenerator::GetNewLambda() {
  return Option::LambdaPrefix + "lambda_" +
    folly::to<std::string>(++s_idLambda);
}

int CodeGenerator::createNewId(const std::string &key) {
  return ++m_idCounters[key];
}

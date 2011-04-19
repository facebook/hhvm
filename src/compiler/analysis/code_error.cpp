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

#include <compiler/analysis/code_error.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/parser/parser.h>
#include <compiler/construct.h>
#include <compiler/option.h>
#include <util/exception.h>
#include <util/lock.h>

using namespace HPHP::JSON;
using namespace std;
using namespace boost;

namespace HPHP { namespace Compiler {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ErrorInfo);
class ErrorInfo : public JSON::ISerializable {
public:
  ErrorType m_error;
  ConstructPtr m_construct1;
  ConstructPtr m_construct2;
  std::string m_data;

  /**
   * Implements JSON::ISerializable.
   */
  virtual void serialize(JSON::OutputStream &out) const;
};

///////////////////////////////////////////////////////////////////////////////

class CodeErrors : public JSON::ISerializable {
public:
  CodeErrors();
  void clear();

  /**
   * Implements JSON::ISerializable.
   */
  virtual void serialize(JSON::OutputStream &out) const;

  void record(ErrorInfoPtr errorInfo);
  bool exists(ErrorType type) const;
  bool exists() const;

  void saveToFile(const char *filename, bool varWrapper) const;

private:
  static std::vector<const char *> ErrorTexts;
  static std::vector<const char *> &getErrorTexts();

  typedef std::map<ConstructPtr, ErrorInfoPtr> ErrorInfoMap;
  std::vector<ErrorInfoMap> m_errors;
  Mutex m_mutex;
};

static CodeErrors s_code_errors;

///////////////////////////////////////////////////////////////////////////////
// class CodeErrors

std::vector<const char *> CodeErrors::ErrorTexts;
std::vector<const char *> &CodeErrors::getErrorTexts() {
  if (ErrorTexts.empty()) {
    ErrorTexts.resize(ErrorCount);
#define CODE_ERROR_ENTRY(x) ErrorTexts[x] = #x;
#include "compiler/analysis/code_error.inc"
#undef CODE_ERROR_ENTRY
  }
  return ErrorTexts;
}

CodeErrors::CodeErrors() {
  m_errors.resize(ErrorCount);
}

void CodeErrors::clear() {
  m_errors.clear();
  m_errors.resize(ErrorCount);
}

void CodeErrors::record(ErrorInfoPtr errorInfo) {
  ASSERT(errorInfo->m_error >= 0 && errorInfo->m_error < ErrorCount);
  Lock lock(m_mutex);
  m_errors[errorInfo->m_error][errorInfo->m_construct1] = errorInfo;
}

bool CodeErrors::exists(ErrorType type) const {
  return !m_errors[type].empty();
}

bool CodeErrors::exists() const {
  for (unsigned int i = 0; i < m_errors.size(); i++) {
    const ErrorInfoMap &errorMap = m_errors[i];
    if (!errorMap.empty()) return true;
  }
  return false;
}

void ErrorInfo::serialize(JSON::OutputStream &out) const {
  JSON::MapStream ms(out);
  if (m_construct1) {
    ms.add("c1", m_construct1);
  }
  if (m_construct2) {
    ms.add("c2", m_construct2);
  }
  if (!m_data.empty()) {
    ms.add("d", m_data);
  }
  ms.done();
}

void CodeErrors::serialize(JSON::OutputStream &out) const {
  vector<const char *> errorTexts = getErrorTexts();

  unsigned int total = 0;
  for (unsigned int i = 0; i < m_errors.size(); i++) {
    total += m_errors[i].size();
  }

  JSON::ListStream ls(out);
  ls << total;
  ls.next();

  JSON::MapStream ms(out);
  for (unsigned int i = 0; i < m_errors.size(); i++) {
    const ErrorInfoMap &errorMap = m_errors[i];
    if (errorMap.empty()) continue;

    ms.add(errorTexts[i]);
    JSON::ListStream ls2(out);

    for (ErrorInfoMap::const_iterator iter = errorMap.begin();
         iter != errorMap.end(); ++iter) {
      ls2 << iter->second;
    }

    ls2.done();
  }

  ms.done();
  ls.done();
}

void CodeErrors::saveToFile(const char *filename, bool varWrapper) const {
  ofstream f(filename);
  if (f) {
    JSON::OutputStream o(f);
    if (varWrapper) f << "var CodeErrors = ";
    serialize(o);
    if (varWrapper) f << ";\n\n";
    f.close();
  }
}

///////////////////////////////////////////////////////////////////////////////

void ClearErrors() {
  s_code_errors.clear();
}

void Error(ErrorType error, ConstructPtr construct) {
  ErrorInfoPtr errorInfo(new ErrorInfo());
  errorInfo->m_error = error;
  errorInfo->m_construct1 = construct;
  errorInfo->m_data = construct->getText();
  s_code_errors.record(errorInfo);
}

void Error(ErrorType error, ConstructPtr construct1, ConstructPtr construct2) {
  ErrorInfoPtr errorInfo(new ErrorInfo());
  errorInfo->m_error = error;
  errorInfo->m_construct1 = construct1;
  errorInfo->m_construct2 = construct2;
  errorInfo->m_data = construct1->getText();
  s_code_errors.record(errorInfo);
}

void Error(ErrorType error, ConstructPtr construct, const std::string &data) {
  ErrorInfoPtr errorInfo(new ErrorInfo());
  errorInfo->m_error = error;
  errorInfo->m_construct1 = construct;
  errorInfo->m_data = data;
  s_code_errors.record(errorInfo);
}

void SaveErrors(JSON::OutputStream &out) {
  s_code_errors.serialize(out);
}

void SaveErrors(const char *filename, bool varWrapper /* = false */) {
  s_code_errors.saveToFile(filename, varWrapper);
}

void DumpErrors() {
  JSON::OutputStream o(cerr);
  s_code_errors.serialize(o);
}

bool HasError(ErrorType type) {
  return s_code_errors.exists(type);
}

bool HasError() {
  return s_code_errors.exists();
}

///////////////////////////////////////////////////////////////////////////////
}}

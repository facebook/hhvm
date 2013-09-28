/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/construct.h"
#include "hphp/compiler/option.h"
#include "hphp/util/exception.h"
#include "hphp/util/lock.h"

using namespace HPHP::JSON;

namespace HPHP { namespace Compiler {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ErrorInfo);
class ErrorInfo : public JSON::CodeError::ISerializable {
public:
  ErrorType m_error;
  ConstructPtr m_construct1;
  ConstructPtr m_construct2;
  std::string m_data;

  /**
   * Implements JSON::CodeError::ISerializable.
   */
  virtual void serialize(JSON::CodeError::OutputStream &out) const;
};

///////////////////////////////////////////////////////////////////////////////

class CodeErrors : public JSON::CodeError::ISerializable {
public:
  CodeErrors();
  void clear();

  /**
   * Implements JSON::CodeError::ISerializable.
   */
  virtual void serialize(JSON::CodeError::OutputStream &out) const;

  void record(ErrorInfoPtr errorInfo);
  bool exists(ErrorType type) const;
  bool exists() const;

  void saveToFile(AnalysisResultPtr ar,
                  const char *filename, bool varWrapper) const;

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
#include "hphp/compiler/analysis/core_code_error.inc"
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
  assert(errorInfo->m_error >= 0 && errorInfo->m_error < ErrorCount);
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

void ErrorInfo::serialize(JSON::CodeError::OutputStream &out) const {
  JSON::CodeError::MapStream ms(out);
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

void CodeErrors::serialize(JSON::CodeError::OutputStream &out) const {
  vector<const char *> errorTexts = getErrorTexts();

  unsigned int total = 0;
  for (unsigned int i = 0; i < m_errors.size(); i++) {
    total += m_errors[i].size();
  }

  JSON::CodeError::ListStream ls(out);
  ls << total;
  ls.next();

  JSON::CodeError::MapStream ms(out);
  for (unsigned int i = 0; i < m_errors.size(); i++) {
    const ErrorInfoMap &errorMap = m_errors[i];
    if (errorMap.empty()) continue;

    ms.add(errorTexts[i]);
    JSON::CodeError::ListStream ls2(out);

    for (ErrorInfoMap::const_iterator iter = errorMap.begin();
         iter != errorMap.end(); ++iter) {
      ls2 << iter->second;
    }

    ls2.done();
  }

  ms.done();
  ls.done();
}

void CodeErrors::saveToFile(AnalysisResultPtr ar,
                            const char *filename,
                            bool varWrapper) const {
  std::ofstream f(filename);
  if (f) {
    JSON::CodeError::OutputStream o(f, ar);
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
  if (!Option::RecordErrors) return;
  ErrorInfoPtr errorInfo(new ErrorInfo());
  errorInfo->m_error = error;
  errorInfo->m_construct1 = construct;
  errorInfo->m_data = construct->getText();
  s_code_errors.record(errorInfo);
}

void Error(ErrorType error, ConstructPtr construct1, ConstructPtr construct2) {
  if (!Option::RecordErrors) return;
  ErrorInfoPtr errorInfo(new ErrorInfo());
  errorInfo->m_error = error;
  errorInfo->m_construct1 = construct1;
  errorInfo->m_construct2 = construct2;
  errorInfo->m_data = construct1->getText();
  s_code_errors.record(errorInfo);
}

void Error(ErrorType error, ConstructPtr construct, const std::string &data) {
  if (!Option::RecordErrors) return;
  ErrorInfoPtr errorInfo(new ErrorInfo());
  errorInfo->m_error = error;
  errorInfo->m_construct1 = construct;
  errorInfo->m_data = data;
  s_code_errors.record(errorInfo);
}

void SaveErrors(JSON::CodeError::OutputStream &out) {
  s_code_errors.serialize(out);
}

void SaveErrors(AnalysisResultPtr ar,
                const char *filename,
                bool varWrapper /* = false */) {
  s_code_errors.saveToFile(ar, filename, varWrapper);
}

void DumpErrors(AnalysisResultPtr ar) {
  JSON::CodeError::OutputStream o(std::cerr, ar);
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

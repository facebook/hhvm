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

#include <compiler/analysis/code_error.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/parser/parser.h>
#include <compiler/construct.h>
#include <compiler/option.h>
#include <util/db_query.h>
#include <util/db_conn.h>
#include <util/exception.h>
#include <util/logger.h>

using namespace HPHP;
using namespace HPHP::JSON;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// class ErrorInfo

ErrorInfo::ErrorInfo()
  : m_expected(Type::KindOfAny), m_actual(Type::KindOfAny),
    m_suppressed(false) {
}

///////////////////////////////////////////////////////////////////////////////
// class CodeError

CodeError::CodeError(AnalysisResultPtr ar)
  : m_ar(ar), m_verbose(true) {
  m_errors.resize(ErrorCount);
}

void CodeError::record(ConstructPtr self, ErrorType error,
                       ConstructPtr construct1,
                       ConstructPtr construct2 /* = ConstructPtr() */,
                       const char *data /* = NULL */) {
  record(error, construct1, construct2, data, self->isErrorSuppressed(error));
}

void CodeError::record(ErrorType error, ConstructPtr construct1,
                       ConstructPtr construct2 /* = ConstructPtr() */,
                       const char *data /* = NULL */,
                       bool suppressed /* = false */) {
  if (!suppressed) {
    AnalysisResultPtr ar = m_ar.lock();
    if (ar && construct1) {
      const char *file = construct1->getLocation()->file;
      FileScopePtr fileScope = ar->findFileScope(file, false);
      if (fileScope) {
        suppressed = fileScope->isErrorSuppressed(error);
      }
    }
  }

  ErrorInfoPtr errorInfo(new ErrorInfo());
  errorInfo->m_error = error;
  errorInfo->m_construct1 = construct1;
  errorInfo->m_construct2 = construct2;
  if (data) {
    errorInfo->m_data = data;
  } else {
    errorInfo->m_data += construct1->getText();
  }
  errorInfo->m_suppressed = suppressed;
  record(errorInfo);
}

void CodeError::record(ConstructPtr construct, Type::KindOf expected,
                       Type::KindOf actual) {
  ErrorInfoPtr errorInfo(new ErrorInfo());
  errorInfo->m_error = BadTypeConversion;
  errorInfo->m_construct1 = construct;
  errorInfo->m_expected = expected;
  errorInfo->m_actual = actual;
  errorInfo->m_data = construct->getText();
  errorInfo->m_suppressed = construct->isErrorSuppressed(BadTypeConversion);
  record(errorInfo);
}

void CodeError::record(ErrorInfoPtr errorInfo) {
  ASSERT(errorInfo->m_error >= 0 && errorInfo->m_error < ErrorCount);

  ErrorInfoMap &errorMap = m_errors[errorInfo->m_error];
  ErrorInfoMap::const_iterator iter = errorMap.find(errorInfo->m_construct1);
  if (iter == errorMap.end()) {
    errorMap[errorInfo->m_construct1] = errorInfo;
    return;
  }
}

bool CodeError::exists(ErrorType type) const {
  const ErrorInfoMap &errors = m_errors[type];
  for (ErrorInfoMap::const_iterator iter = errors.begin();
       iter != errors.end(); ++iter) {
    if (!iter->second->m_suppressed) {
      return true;
    }
  }
  return false;
}

bool CodeError::exists(bool verbose) const {
  m_verbose = verbose;
  for (unsigned int i = 0; i < m_errors.size(); i++) {
    const ErrorInfoMap &errorMap = m_errors[i];
    if (errorMap.empty()) continue;
    if (filtered(i)) continue;
    m_verbose = true;
    return true;
  }
  m_verbose = true;
  return false;
}

///////////////////////////////////////////////////////////////////////////////

std::vector<const char *> CodeError::ErrorTexts;
std::vector<const char *> &CodeError::getErrorTexts() {
  if (ErrorTexts.empty()) {
    ErrorTexts.resize(ErrorCount);
#define CODE_ERROR_ENTRY(x) ErrorTexts[x] = #x;
#include "compiler/analysis/code_error.inc"
#undef CODE_ERROR_ENTRY
  }
  return ErrorTexts;
}

void ErrorInfo::serialize(JSON::OutputStream &out) const {
  out.raw() << "{";
  if (m_construct1) {
    out << Name("c1") << m_construct1; out.raw() << ',';
  }
  if (m_construct2) {
    out << Name("c2") << m_construct2; out.raw() << ',';
  }
  if (m_expected != Type::KindOfSome && m_expected != Type::KindOfAny) {
    out << Name("et") << m_expected; out.raw() << ',';
  }
  if (m_actual != Type::KindOfSome && m_actual != Type::KindOfAny) {
    out << Name("at") << m_actual; out.raw() << ',';
  }
  if (!m_data.empty()) {
    out << Name("d") << m_data; out.raw() << ',';
  }
  out << Name("s") << m_suppressed; out.raw() << ',';
  out.raw() << "}\n";
}

void CodeError::serialize(JSON::OutputStream &out) const {
  vector<const char *> errorTexts = getErrorTexts();

  unsigned int total = 0;
  for (unsigned int i = 0; i < m_errors.size(); i++) {
    total += m_errors[i].size();
  }

  out.raw() << "["; out << total; out.raw() << ", {\n";
  bool comma = false;
  for (unsigned int i = 0; i < m_errors.size(); i++) {
    const ErrorInfoMap &errorMap = m_errors[i];
    if (errorMap.empty()) continue;
    if (filtered(i)) continue;

    if (comma) {
      out.raw() << ',';
    } else {
      comma = true;
    }
    out << Name(errorTexts[i]);
    out.raw() << "\n[";
    for (ErrorInfoMap::const_iterator iter = errorMap.begin();
         iter != errorMap.end(); ++iter) {
      if (iter != errorMap.begin()) out.raw() << ',';
      out << iter->second;
    }
    out.raw() << "]\n";
  }
  out.raw() << "}]\n";
}

void CodeError::dump(bool verbose) const {
  m_verbose = verbose;
  JSON::OutputStream o(cerr);
  serialize(o);
  m_verbose = true;
}

void CodeError::saveToFile(const char *filename, bool verbose,
                           bool varWrapper /* = false */) const {
  ofstream f(filename);
  if (f) {
    m_verbose = verbose;
    JSON::OutputStream o(f);
    if (varWrapper) f << "var CodeErrors = ";
    serialize(o);
    if (varWrapper) f << ";\n\n";
    f.close();
    m_verbose = true;
  }
}

void CodeError::saveToDB(ServerDataPtr server, int runId) const {
  vector<const char *> errorTexts = getErrorTexts();

  DBConn conn;
  conn.open(server);

  const char *sql = "INSERT INTO hphp_err (run, program, kind, construct, "
    "file1, line1, file2, line2, expected_type, actual_type, data, "
    "suppressed)";
  DBQueryPtr q(new DBQuery(&conn, sql));
  int count = 0;
  const int MAX_COUNT = 1000;

  for (unsigned int i = 0; i < m_errors.size(); i++) {
    const ErrorInfoMap &errorMap = m_errors[i];
    for (ErrorInfoMap::const_iterator iter = errorMap.begin();
         iter != errorMap.end(); ++iter) {
      ErrorInfo &err = *iter->second;

      const char *file1 = "";
      const char *file2 = "";
      int line1 = 0;
      int line2 = 0;
      if (err.m_construct1) {
        file1 = err.m_construct1->getLocation()->file;
        line1 = err.m_construct1->getLocation()->line1;
      }
      if (err.m_construct2) {
        file2 = err.m_construct2->getLocation()->file;
        line2 = err.m_construct2->getLocation()->line1;
      }

      q->insert("%d,'%s','%s', %p, '%s',%d,'%s',%d, %d,%d,'%s', %d",
                runId, "", errorTexts[i],
                err.m_construct1.get(), file1, line1, file2, line2,
                err.m_expected, err.m_actual, err.m_data.c_str(),
                (err.m_suppressed ? 1 : 0));
      if (++count >= MAX_COUNT) {
        count = 0;
        q->execute();
        q = DBQueryPtr(new DBQuery(&conn, sql));
      }
    }
  }

  if (count) q->execute();
}

bool CodeError::lookupErrorType(std::string error, ErrorType &r) {
#define CODE_ERROR_ENTRY(x)                     \
  if(error == #x) {                             \
    r = x;                                      \
    return true;                                \
  } else
#include <compiler/analysis/code_error.inc>
#undef CODE_ERROR_ENTRY
  {
    return false;
  }
}

bool CodeError::filtered(int error) const {
  if (!m_verbose) {
    switch (error) {
    case BadDefine:
    case BadPHPIncludeFile:
    case BadPassByReference:
    case PHPIncludeFileNotFound:
    case UseEvaluation:
    case UnknownClass:
    case UnknownBaseClass:
    case UnknownFunction:
    case DeclaredClassTwice:
    case DeclaredFunctionTwice:
    case DeclaredStaticVariableTwice:
    case DeclaredVariableTwice:
    case RedundantParameter:
    case StatementHasNoEffect:
    case InvalidArrayElement:
    case InvalidDerivation:
    case MissingAbstractMethodImpl:
    case ReassignThis:
      return false;
    default:
      return true;
    }
  }
  return false;
}

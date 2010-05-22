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

#ifndef __CODE_ERROR_H__
#define __CODE_ERROR_H__

#include <compiler/hphp.h>
#include <util/json.h>
#include <compiler/analysis/type.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ServerData);
DECLARE_BOOST_TYPES(Construct);
DECLARE_BOOST_TYPES(ErrorInfo);
DECLARE_BOOST_TYPES(CodeError);

class CodeError : public JSON::ISerializable {
public:
  enum ErrorType {
#define CODE_ERROR_ENTRY(x) x,
#include "compiler/analysis/code_error.inc"
#undef CODE_ERROR_ENTRY
    ErrorCount
  };

public:
  CodeError(AnalysisResultPtr ar);

  /**
   * Record a coding error within the AST
   */
  void record(ConstructPtr self, ErrorType error, ConstructPtr construct1,
              ConstructPtr construct2 = ConstructPtr(),
              const char *data = NULL);
  /**
   * Record a coding error outside the AST
   */
  void record(ErrorType error, ConstructPtr construct1,
              ConstructPtr construct2 = ConstructPtr(),
              const char *data = NULL,
              bool suppressed = false);

  /**
   * Record a BadTypeConversion error.
   */
  void record(ConstructPtr construct, Type::KindOf expected,
              Type::KindOf actual);

  /**
   * Implements JSON::ISerializable.
   */
  virtual void serialize(JSON::OutputStream &out) const;

  /**
   * Dump JavaScript output to stderr.
   */
  void dump(bool verbose) const;

  /**
   * Save JavaScript output to specified file.
   */
  void saveToFile(const char *filename, bool verbose,
                  bool varWrapper = false) const;

  /**
   * Save code error to a database.
   */
  void saveToDB(ServerDataPtr server, int runId) const;

  /**
   * Whether specified type of error is present. Written for unit test.
   */
  bool exists(ErrorType type) const;
  bool exists(bool verbose) const; // any error

  static bool lookupErrorType(std::string error, ErrorType &result);

private:
  boost::weak_ptr<AnalysisResult> m_ar;
  static std::vector<const char *> ErrorTexts;
  static std::vector<const char *> &getErrorTexts();

  typedef std::map<ConstructPtr, ErrorInfoPtr> ErrorInfoMap;
  std::vector<ErrorInfoMap> m_errors;
  mutable bool m_verbose;

  void record(ErrorInfoPtr errorInfo);
  bool filtered(int error) const;
};

class ErrorInfo : public JSON::ISerializable {
public:
  ErrorInfo();

  CodeError::ErrorType m_error;
  ConstructPtr m_construct1;
  ConstructPtr m_construct2;
  Type::KindOf m_expected;
  Type::KindOf m_actual;
  std::string m_data;
  bool m_suppressed;

  /**
   * Implements JSON::ISerializable.
   */
  virtual void serialize(JSON::OutputStream &out) const;
};
///////////////////////////////////////////////////////////////////////////////
}

#endif // __CODE_ERROR_H__

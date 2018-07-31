/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include <boost/variant.hpp>

#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/unit-emitter.h"

namespace HPHP {

struct MD5;

struct BadCompilerException : Exception {
  explicit BadCompilerException(const std::string& what) : Exception(what) {}
  template<class... A>
  explicit BadCompilerException(A&&... args)
    : Exception(folly::sformat(std::forward<A>(args)...))
  {}
};

void compilers_start();
void compilers_shutdown();
void compilers_set_user(const std::string& username);
void compilers_detach_after_fork();

// On success return a verified unit, and on failure return a string stating the
// type of error encountered
using CompilerResult = boost::variant<std::unique_ptr<UnitEmitter>,std::string>;

struct FactsParser {
  virtual ~FactsParser() {
  }
};

std::unique_ptr<FactsParser> acquire_facts_parser();

struct FactsJSONString {
  std::string value;
};

// On success returns a Json with value containing json-serialized results of
// facts extraction and on failure returns a string with error text
using ParseFactsResult = boost::variant<FactsJSONString, std::string>;

ParseFactsResult extract_facts(const FactsParser&,
                               const std::string& filename,
                               const char* code,
                               int len);

std::string hackc_version();

struct UnitCompiler {
  UnitCompiler(const char* code,
               int codeLen,
               const char* filename,
               const MD5& md5,
               bool forDebuggerEval)
      : m_code(code),
        m_codeLen(codeLen),
        m_filename(filename),
        m_md5(md5),
        m_forDebuggerEval(forDebuggerEval)
    {}
  virtual ~UnitCompiler() {}

  static std::unique_ptr<UnitCompiler> create(
    const char* code,
    int codeLen,
    const char* filename,
    const MD5& md5,
    bool forDebuggerEval);
  virtual std::unique_ptr<UnitEmitter> compile(
    AsmCallbacks* callbacks = nullptr) const = 0;
  virtual const char* getName() const = 0;

 protected:
  const char* m_code;
  int m_codeLen;
  const char* m_filename;
  const MD5& m_md5;
  bool m_forDebuggerEval;
};

struct HackcUnitCompiler : public UnitCompiler {
  using UnitCompiler::UnitCompiler;

  virtual std::unique_ptr<UnitEmitter> compile(
    AsmCallbacks* callbacks = nullptr) const override;
  virtual const char* getName() const override { return "HackC"; }
};

}

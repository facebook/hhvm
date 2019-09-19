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
#include <folly/Range.h>

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/unit-emitter.h"

namespace HPHP {

namespace Native {
struct FuncTable;
}

struct SHA1;

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

struct FfpJSONString {
  std::string value;
};

// On success returns a Json with value containing json-serialized results of
// facts extraction and on failure returns a string with error text
using ParseFactsResult = boost::variant<FactsJSONString, std::string>;
using FfpResult = boost::variant<FfpJSONString, std::string>;

ParseFactsResult extract_facts(const FactsParser&,
                               const std::string& filename,
                               const char* code,
                               int len,
                               const RepoOptions& options);
FfpResult ffp_parse_file(std::string file,
                         const char* contents,
                         int size,
                         const RepoOptions& options);

std::string hackc_version();

struct UnitCompiler {
  UnitCompiler(const char* code,
               int codeLen,
               const char* filename,
               const SHA1& sha1,
               const Native::FuncTable& nativeFuncs,
               bool forDebuggerEval,
               const RepoOptions& options)
      : m_code(code),
        m_codeLen(codeLen),
        m_filename(filename),
        m_sha1(sha1),
        m_nativeFuncs(nativeFuncs),
        m_forDebuggerEval(forDebuggerEval),
        m_options(options)
    {}
  virtual ~UnitCompiler() {}

  static std::unique_ptr<UnitCompiler> create(
    const char* code,
    int codeLen,
    const char* filename,
    const SHA1& sha1,
    const Native::FuncTable& nativeFuncs,
    bool forDebuggerEval,
    const RepoOptions& options);

  virtual std::unique_ptr<UnitEmitter> compile(
    bool wantsSymbolRefs = false) const = 0;

  virtual const char* getName() const = 0;

 protected:
  const char* m_code;
  int m_codeLen;
  const char* m_filename;
  const SHA1& m_sha1;
  const Native::FuncTable& m_nativeFuncs;
  bool m_forDebuggerEval;
  const RepoOptions& m_options;
};

struct HackcUnitCompiler : public UnitCompiler {
  using UnitCompiler::UnitCompiler;

  virtual std::unique_ptr<UnitEmitter> compile(
    bool wantsSymbolRefs = false) const override;

  virtual const char* getName() const override { return "HackC"; }
};

using HhasHandler = std::string (*)(
  const char*,
  const SHA1&,
  folly::StringPiece::size_type,
  StructuredLogEntry&,
  std::function<void(const char *)>,
  std::function<std::string()>,
  const RepoOptions&
);

extern HhasHandler g_hhas_handler;
}

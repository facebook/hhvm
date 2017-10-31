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

enum class HackcMode {
  kNever,
  kFallback,
  kFatal
};

HackcMode hackc_mode();

void compilers_init();
void compilers_start();
void compilers_shutdown();
void compilers_set_user(const std::string& username);

// On success return a verified unit, and on failure return a string stating the
// type of error encountered
using CompilerResult = boost::variant<std::unique_ptr<UnitEmitter>,std::string>;

CompilerResult hackc_compile(const char* code,
                             int len,
                             const char* filename,
                             const MD5& md5,
                             AsmCallbacks* callbacks = nullptr);
CompilerResult php7_compile(const char* code,
                            int len,
                            const char* filename,
                            const MD5& md5,
                            AsmCallbacks* callbacks = nullptr);
std::string hackc_version();
std::string php7c_version();

struct UnitCompiler {
  UnitCompiler(const char* code,
               int codeLen,
               const char* filename,
               const MD5& md5)
      : m_code(code),
        m_codeLen(codeLen),
        m_filename(filename),
        m_md5(md5)
    {}
  virtual ~UnitCompiler() {}

  static std::unique_ptr<UnitCompiler> create(
    const char* code,
    int codeLen,
    const char* filename,
    const MD5& md5);
  virtual std::unique_ptr<UnitEmitter> compile(
    AsmCallbacks* callbacks = nullptr) const = 0;
  virtual const char* getName() const = 0;

 protected:
  const char* m_code;
  int m_codeLen;
  const char* m_filename;
  const MD5& m_md5;
};

struct Php7UnitCompiler : public UnitCompiler {
  using UnitCompiler::UnitCompiler;
  virtual std::unique_ptr<UnitEmitter> compile(
    AsmCallbacks* callbacks = nullptr) const override;
  virtual const char* getName() const override { return "PHP7"; }
};

struct HackcUnitCompiler : public UnitCompiler {
  HackcUnitCompiler(const char* code,
                    int codeLen,
                    const char* filename,
                    const MD5& md5,
                    HackcMode hackcMode)
      : UnitCompiler(code, codeLen, filename, md5),
        m_hackcMode(hackcMode)
    {}

  virtual std::unique_ptr<UnitEmitter> compile(
    AsmCallbacks* callbacks = nullptr) const override;
  virtual const char* getName() const override { return "HackC"; }

 private:
  const HackcMode m_hackcMode;
};

}

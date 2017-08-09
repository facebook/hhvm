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

#include "hphp/php7/ast_dump.h"
#include "hphp/php7/compiler.h"
#include "hphp/php7/zend/zend_language_scanner_defs.h"
#include "hphp/php7/bytecode.h"
#include "hphp/php7/hhas.h"

#include <folly/Format.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <string>
#include <iostream>

namespace {

using HPHP::php7::CompilerException;
using HPHP::php7::LanguageException;
using HPHP::php7::compile;
using HPHP::php7::dump_asm;
using HPHP::php7::makeFatalUnit;

struct Options {
  bool daemonEnabled{false};
};

static constexpr size_t read_chunk = 1024;
static constexpr size_t allocate_chunk = 4096;

std::unique_ptr<folly::IOBuf> readAll(std::istream& in) {
  folly::IOBufQueue queue;

  // load data from the file into memory
  while (!in.eof()) {
    size_t realsize;
    void* buf;
    std::tie(buf, realsize) = queue.preallocate(read_chunk, allocate_chunk);
    in.read(static_cast<char*>(buf), realsize);
    size_t written = in.gcount();
    queue.postallocate(written);
  }

  // add the tailroom the lexer expects
  void* tail = queue.allocate(ZEND_MMAP_AHEAD);
  memset(tail, 0, ZEND_MMAP_AHEAD);

  // coalesce and return the buffer
  auto buf = queue.move();
  buf->coalesce();
  buf->trimEnd(ZEND_MMAP_AHEAD);
  return buf;
}

zend_ast* runParser(const folly::IOBuf& buffer) {
  init_parser_state();
  startup_scanner();

  unsigned char* buf =
    const_cast<unsigned char*>(buffer.data());
  LANG_SCNG(yy_cursor) = buf;
  LANG_SCNG(yy_limit) = buf + buffer.length();
  LANG_SCNG(yy_start) = buf;
  LANG_SCNG(yy_state) = yycINITIAL;
  CG(ast_arena) = zend_arena_create(256);

  zendparse();
  // dump AST to stderr
  HPHP::php7::dump_ast(std::cerr, CG(ast));
  std::cerr << std::endl;
  return CG(ast);
}

std::string runCompiler(const std::string& filename, const folly::IOBuf& buf) {
  try {
    auto ast = runParser(buf);
    auto unit = compile(filename, ast);
    return dump_asm(*unit);
  } catch (const LanguageException& e) {
    return dump_asm(*makeFatalUnit(filename, e.what()));
  }
}

int runDaemon() {
  while (true) {
    try {
      std::string filename;
      std::string md5;
      std::string length;

      std::getline(std::cin, filename);
      std::getline(std::cin, md5);
      std::getline(std::cin, length);

      size_t code_length;
      try {
        code_length = std::stoul(length);
      } catch (...) {
        throw std::runtime_error("Could not read code length from stdin");
      }

      auto buf = folly::IOBuf::create(code_length + ZEND_MMAP_AHEAD);
      if (!std::cin.read(
            reinterpret_cast<char*>(buf->writableData()),
            code_length)) {
        throw std::runtime_error("Could not read code from stdin");
      }
      memset(buf->writableData() + code_length, '\0', ZEND_MMAP_AHEAD);
      buf->append(code_length + ZEND_MMAP_AHEAD);

      auto hhas = runCompiler(filename, *buf);
      std::cout << hhas.length() << std::endl
                << hhas << std::endl;
    } catch (const CompilerException& e) {
      std::cout << "ERROR: " << e.what() << std::endl;
    }
  }
}

void parseFlags(int argc, char** argv, Options& opts) {
  for (int i = 1; i < argc; i++) {
    const char* s = argv[i];
    if (strcmp(s, "--daemon") == 0) {
      opts.daemonEnabled = true;
    } else {
      throw std::runtime_error(folly::sformat("Unrecognized flag: {}", s));
    }
  }
}

} // namespace

int main(int argc, char** argv) {
  Options opts;

  try {
    parseFlags(argc, argv, opts);

    if (opts.daemonEnabled) {
      return runDaemon();
    } else {
      auto buf = readAll(std::cin);
      std::cout << runCompiler("stdin.php", *buf) << std::endl;
    }
  } catch (const std::runtime_error& e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 2;
  }

  return 0;
}

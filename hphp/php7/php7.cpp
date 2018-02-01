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
#include "hphp/php7/options.h"
#include "hphp/php7/zend/zend.h"

#include "hphp/util/embedded-data.h"

#include <folly/dynamic.h>
#include <folly/Format.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/json.h>

#include <string>
#include <iostream>

namespace HPHP {
Options g_opts;
}

namespace {

using HPHP::php7::CompilerException;
using HPHP::php7::LanguageException;
using HPHP::php7::compile;
using HPHP::php7::dump_asm;
using HPHP::php7::makeFatalUnit;

std::string getBuildId() {
  HPHP::embedded_data data;
  auto constexpr unknown_version = "php7-unknown-version";
  if (!HPHP::get_embedded_data("build_id", &data)) {
    return unknown_version;
  }

  auto const str = HPHP::read_embedded_data(data);
  if (str.empty()) {
    return unknown_version;
  }

  return str;
}

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
  // PHP allows the cursor to be advanced to yy_limit + ZEND_MMAP_AHEAD, so
  // don't tell it about our extra allocated space.
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

  if (HPHP::g_opts.dumpAst) {
    HPHP::php7::dump_ast(std::cerr, CG(ast));
    std::cerr << std::endl;
  }

  return CG(ast);
}

std::string runCompiler(const std::string& filename, const folly::IOBuf& buf) {
  try {
    auto ast = runParser(buf);
    auto unit = compile(filename, ast);
    return dump_asm(*unit);
  } catch (const LanguageException& e) {
    return dump_asm(*makeFatalUnit(filename, e.what()));
  } catch (const ZendException& e) {
    return dump_asm(*makeFatalUnit(filename, e.what()));
  }
}

void writeMessage(
  folly::dynamic& header,
  folly::StringPiece body
) {
  header["bytes"] = body.size();
  std::cout
    << folly::toJson(header) << std::endl
    << body << std::flush;
}

void writeVersion() {
  folly::dynamic header = folly::dynamic::object
    ("type", "compiler_version")
    ("version", getBuildId());
  writeMessage(header, "");
}

void writeProgram(
  folly::StringPiece filename,
  folly::StringPiece code
) {
  folly::dynamic header = folly::dynamic::object
    ("type", "hhas")
    ("file", filename);
  writeMessage(header, code);
}

void writeError(
  folly::StringPiece filename,
  folly::StringPiece error
) {
  folly::dynamic header = folly::dynamic::object
    ("type", "error")
    ("file", filename)
    ("error", error);
  writeMessage(header, "");
}

int processMessage() {
  std::string headerStr;
  std::getline(std::cin, headerStr);
  if (!std::cin.good()) {
    return 2;
  }
  if (headerStr.empty()) {
    return 0;
  }

  const auto header = folly::parseJson(headerStr);
  const std::string type = header.at("type").asString();
  const std::size_t bytes = header.getDefault("bytes", "0").asInt();
  const std::string filename =
    header.getDefault("file", "[unknown]").asString();

  if (type == "code") {
    auto buf = folly::IOBuf::create(bytes + ZEND_MMAP_AHEAD);
    if (!std::cin.read(reinterpret_cast<char*>(buf->writableData()), bytes)) {
      throw std::runtime_error("Could not read code from stdin");
    }
    // PHP allows the cursor to be advanced to yy_limit + ZEND_MMAP_AHEAD, so
    // don't tell it about our extra allocated space.
    memset(buf->writableData() + bytes, '\0', ZEND_MMAP_AHEAD);
    buf->append(bytes);
    try {
      const auto hhas = runCompiler(filename, *buf);
      writeProgram(filename, hhas);
    } catch (const CompilerException& e) {
      writeError(filename, e.what());
    }
  } else if (type == "error") {
    throw std::runtime_error(header.at("error").asString());
  } else {
    // Else we don't know how to process this type (yet?). So just read through
    // it and continue.
    auto buf = folly::IOBuf::create(bytes);
    if (!std::cin.read(reinterpret_cast<char*>(buf->writableData()), bytes)) {
      throw std::runtime_error("Could not message body from stdin.");
    }
  }

  return 0;
}

int runDaemon() {
  int ret;
  writeVersion();
  while (!(ret = processMessage()));
  return ret;
}

void parseFlags(int argc, char** argv) {
  for (int i = 1; i < argc; i++) {
    const char* s = argv[i];
    if (strcmp(s, "--daemon") == 0) {
      HPHP::g_opts.daemonEnabled = true;
    } else if (strcmp(s, "--lineno") == 0) {
      HPHP::g_opts.linenoEnabled = true;
    } else if (strcmp(s, "--dump-ast") == 0) {
      HPHP::g_opts.dumpAst = true;
    } else {
      throw std::runtime_error(folly::sformat("Unrecognized flag: {}", s));
    }
  }
}

} // namespace

int main(int argc, char** argv) {
  try {
    parseFlags(argc, argv);

    if (HPHP::g_opts.daemonEnabled) {
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

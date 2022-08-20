/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>
#include <folly/io/IOBuf.h>
#include <sstream>
#include <string>

namespace proxygen {

class IOBufPrinter {
 public:
  enum class Format : uint8_t {
    HEX_FOLLY = 0,
    HEX_16 = 1,
    CHAIN_INFO = 2,
    BIN = 3,
  };

  static std::string printChain(const folly::IOBuf* buf,
                                Format format,
                                bool coalesce);

  static std::string printHexFolly(const folly::IOBuf* buf,
                                   bool coalesce = false) {
    return printChain(buf, Format::HEX_FOLLY, coalesce);
  }

  static std::string printHex16(const folly::IOBuf* buf,
                                bool coalesce = false) {
    return printChain(buf, Format::HEX_16, coalesce);
  }

  static std::string printChainInfo(const folly::IOBuf* buf) {
    return printChain(buf, Format::CHAIN_INFO, false);
  }

  static std::string printBin(const folly::IOBuf* buf, bool coalesce = false) {
    return printChain(buf, Format::BIN, coalesce);
  }

  IOBufPrinter() {
  }
  virtual ~IOBufPrinter() {
  }

  virtual std::string print(const folly::IOBuf* buf) = 0;
};

class Hex16Printer : public IOBufPrinter {
 public:
  std::string print(const folly::IOBuf* buf) override;
};

class HexFollyPrinter : public IOBufPrinter {
 public:
  std::string print(const folly::IOBuf* buf) override;
};

class ChainInfoPrinter : public IOBufPrinter {
 public:
  std::string print(const folly::IOBuf* buf) override;
};

class BinPrinter : public IOBufPrinter {
 public:
  std::string print(const folly::IOBuf* buf) override;
};

/**
 * write the entire binary content from all the buffers into a binary file
 */
void dumpBinToFile(const std::string& filename, const folly::IOBuf* buf);

/**
 * helper functions for printing in hex a byte array
 * see unit test for example
 */
std::string hexStr(folly::StringPiece sp);

} // namespace proxygen

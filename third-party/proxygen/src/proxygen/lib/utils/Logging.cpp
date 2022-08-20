/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/Logging.h>

#include <folly/Format.h>
#include <folly/Singleton.h>
#include <folly/String.h>
#include <folly/experimental/symbolizer/Symbolizer.h>
#include <fstream>
#include <memory>
#include <ostream>
#include <sstream>
#include <sys/stat.h>
#include <vector>

using folly::IOBuf;
using folly::StringPiece;
using std::string;
using std::stringstream;
using std::unique_ptr;
using std::vector;

namespace {
proxygen::HexFollyPrinter hexFollyPrinter;
proxygen::Hex16Printer hex16Printer;
proxygen::ChainInfoPrinter chainInfoPrinter;
proxygen::BinPrinter binPrinter;

vector<proxygen::IOBufPrinter*> printers = {
    &hexFollyPrinter, &hex16Printer, &chainInfoPrinter, &binPrinter};
} // namespace

namespace proxygen {

string hexStr(StringPiece sp) {
  string out;
  for (auto ch : sp) {
    out.append(folly::sformat("{:02x}", (uint8_t)ch));
  }
  return out;
}

string HexFollyPrinter::print(const IOBuf* buf) {
  return folly::hexDump(buf->data(), buf->length());
}

string Hex16Printer::print(const IOBuf* buf) {
  stringstream out;
  const uint8_t* data = buf->data();
  char tmp[24];
  for (size_t i = 0; i < buf->length(); i++) {
    snprintf(tmp, 3, "%02x", data[i]);
    out << tmp;
    if ((i + 1) % 2 == 0) {
      out << ' ';
    }
    if ((i + 1) % 16 == 0) {
      out << std::endl;
    }
  }
  return out.str();
}

string ChainInfoPrinter::print(const IOBuf* buf) {
  stringstream out;
  out << "iobuf of size " << buf->length() << " tailroom " << buf->tailroom();
  return out.str();
}

string BinPrinter::print(const IOBuf* buf) {
  static uint8_t bytesPerLine = 8;
  string out;
  const uint8_t* data = buf->data();
  for (size_t i = 0; i < buf->length(); i++) {
    for (int b = 7; b >= 0; b--) {
      out += data[i] & 1 << b ? '1' : '0';
    }
    out += ' ';
    out += isprint(data[i]) ? data[i] : ' ';
    if ((i + 1) % bytesPerLine == 0) {
      out += '\n';
    } else {
      out += ' ';
    }
  }
  out += '\n';
  return out;
}

string IOBufPrinter::printChain(const IOBuf* buf,
                                Format format,
                                bool coalesce) {
  uint8_t index = (uint8_t)format;
  if (printers.size() <= index) {
    LOG(ERROR) << "invalid format: " << index;
    return "";
  }
  auto printer = printers[index];
  // empty chain
  if (!buf) {
    return "";
  }

  unique_ptr<IOBuf> cbuf = nullptr;
  if (coalesce) {
    cbuf = buf->clone();
    cbuf->coalesce();
    buf = cbuf.get();
  }
  auto b = buf;
  string res;
  do {
    res += printer->print(b);
    b = b->next();
  } while (b != buf);
  return res;
}

void dumpBinToFile(const string& filename, const IOBuf* buf) {
  struct stat fstat;
  bool exists = (stat(filename.c_str(), &fstat) == 0);
  if (exists) {
    // don't write anything if the file exists
    return;
  }
  std::ofstream file(filename, std::ofstream::binary);
  if (!file.is_open()) {
    LOG(ERROR) << "cannot open file " << filename;
    return;
  }
  if (!buf) {
    file.close();
    return;
  }
  const IOBuf* first = buf;
  do {
    file.write((const char*)buf->data(), buf->length());
    buf = buf->next();
  } while (buf != first);
  file.close();
  LOG(INFO) << "wrote chain " << IOBufPrinter::printChainInfo(buf) << " to "
            << filename;
}
} // namespace proxygen

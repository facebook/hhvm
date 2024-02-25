/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/experimental/RFC1867.h>
#include <proxygen/lib/utils/Logging.h>

using folly::IOBuf;
using folly::IOBufQueue;
using folly::StringPiece;
using folly::io::Cursor;
using std::string;

namespace {
// This is required to get HTTP1xCodec ready to parse a header block
const string kDummyGet("GET / HTTP/1.0");

enum class BoundaryResult { YES, NO, PARTIAL };

BoundaryResult isBoundary(const IOBuf& buf,
                          uint32_t offset,
                          char const* boundary,
                          size_t boundarylen) {
  assert(offset <= buf.length());
  const IOBuf* crtBuf = &buf;
  do {
    size_t crtLen = crtBuf->length() - offset;
    const uint8_t* crtData = crtBuf->data() + offset;
    size_t cmplen = std::min(crtLen, boundarylen);
    if (memcmp(crtData, boundary, cmplen) == 0) {
      if (cmplen == boundarylen) {
        return BoundaryResult::YES;
      } else {
        // beginning of a partial match
        boundary += cmplen;
        boundarylen -= cmplen;
      }
    } else {
      return BoundaryResult::NO;
    }
    offset = 0;
    crtBuf = crtBuf->next();
  } while (crtBuf != &buf);

  return BoundaryResult::PARTIAL;
}

} // namespace

namespace proxygen {

std::unique_ptr<IOBuf> RFC1867Codec::onIngress(std::unique_ptr<IOBuf> data) {
  static auto dummyBuf =
      IOBuf::wrapBuffer(kDummyGet.data(), kDummyGet.length());
  IOBufQueue result{IOBufQueue::cacheChainLength()};
  bool foundBoundary = false;
  BoundaryResult br = BoundaryResult::NO;

  input_.append(std::move(data));
  while (!input_.empty()) {
    switch (state_) {
      case ParserState::START:
        // first time, must start with boundary without leading \n
        br = isBoundary(
            *input_.front(), 0, boundary_.data() + 1, boundary_.length() - 1);
        if (br == BoundaryResult::NO) {
          if (callback_) {
            LOG(ERROR) << "Invalid starting sequence";
            callback_->onError();
          }
          state_ = ParserState::ERROR;
          return nullptr;
        } else if (br == BoundaryResult::PARTIAL) {
          return input_.move();
        }
        input_.trimStart(boundary_.length() - 1);
        bytesProcessed_ += boundary_.length() - 1;
        state_ = ParserState::HEADERS_START;
        [[fallthrough]];

      case ParserState::HEADERS_START: {
        if (input_.chainLength() < 3) {
          return input_.move();
        }
        Cursor c(input_.front());
        char firstTwo[2];
        c.pull(firstTwo, 2);
        // We have at least 3 chars available to read
        uint8_t toTrim = 3;
        if (memcmp(firstTwo, "--", 2) == 0) {
          do {
            auto ch = c.read<char>();
            if (ch == '\n') {
              input_.trimStart(toTrim);
              state_ = ParserState::DONE;
            } else if (ch == '\r') {
              // Every \r we encounter is a char we must trim but we must
              // make sure we have sufficient data available in input_ to
              // keep reading (toTrim is always one pos ahead to handle the
              // expected \n)
              ++toTrim;
              if (input_.chainLength() < toTrim) {
                return input_.move();
              }
            } else {
              state_ = ParserState::ERROR;
            }
          } while (state_ == ParserState::HEADERS_START);
          break;
        }
      }
        headerParser_.setParserPaused(false);
        headerParser_.onIngress(*dummyBuf);
        CHECK(!parseError_);
        state_ = ParserState::HEADERS;
        [[fallthrough]];

      case ParserState::HEADERS:
        while (!parseError_ && input_.front() &&
               state_ == ParserState::HEADERS) {
          size_t bytesParsed = headerParser_.onIngress(*input_.front());
          input_.trimStart(bytesParsed);
          bytesProcessed_ += bytesParsed;
        }
        if (parseError_) {
          if (callback_) {
            LOG(ERROR) << "Error parsing header data: ";
            VLOG(3) << IOBufPrinter::printHexFolly(input_.front());
            callback_->onError();
          }
          state_ = ParserState::ERROR;
          return nullptr;
        }
        break;

      case ParserState::FIELD_DATA:
        result = readToBoundary(foundBoundary);
        value_.append(result.move());
        if (!value_.empty() && callback_) {
          if (callback_->onFieldData(value_.move(), bytesProcessed_) < 0) {
            LOG(ERROR) << "Callback returned error";
            state_ = ParserState::ERROR;
            return nullptr;
          }
        }
        if (foundBoundary) {
          if (callback_) {
            callback_->onFieldEnd(true, bytesProcessed_);
          }
          state_ = ParserState::HEADERS_START;
        } else {
          if (input_.chainLength() > 0) {
            VLOG(5) << "Trailing input="
                    << IOBufPrinter::printHexFolly(input_.front());
          }
          return input_.move();
        }
        break;
      case ParserState::DONE:
      case ParserState::ERROR:
        // abort, consume all input
        return nullptr;
    }
  }
  return nullptr;
}

void RFC1867Codec::onHeadersComplete(HTTPCodec::StreamID /*stream*/,
                                     std::unique_ptr<HTTPMessage> msg) {
  static const StringPiece kName("name", 4);
  static const StringPiece kFilename("filename", 8);
  static const StringPiece kFormData("form-data", 9);

  const auto& contentDisp =
      msg->getHeaders().getSingleOrEmpty(HTTP_HEADER_CONTENT_DISPOSITION);
  string name;
  folly::Optional<string> filename; // filename is optional
  HTTPMessage::splitNameValuePieces(
      contentDisp,
      ';',
      '=',
      [&](folly::StringPiece parameter, folly::StringPiece value) {
        // TODO: Trim whitespace first
        // Strip quotes if present
        if (value.size() >= 2 && value[0] == '\"' &&
            value[value.size() - 1] == '\"') {
          value.reset(value.data() + 1, value.size() - 2);
        }
        if (parameter == kName) {
          name = value.str();
        } else if (parameter == kFilename) {
          filename = value.str();
        } else if (parameter != kFormData) {
          LOG(WARNING) << "Ignoring parameter " << parameter << " value \""
                       << value << '"';
        }
      });
  if (name.empty()) {
    if (callback_) {
      LOG(ERROR) << "name empty";
      callback_->onError();
    }
    state_ = ParserState::ERROR;
    return;
  } else {
    state_ = ParserState::FIELD_DATA;
    if (callback_ && callback_->onFieldStart(
                         name, filename, std::move(msg), bytesProcessed_) < 0) {
      field_ = name;
      LOG(WARNING) << "Callback returned error";
      state_ = ParserState::ERROR;
    }
  }
}

IOBufQueue RFC1867Codec::readToBoundary(bool& foundBoundary) {
  IOBufQueue result{IOBufQueue::cacheChainLength()};
  BoundaryResult boundaryResult = BoundaryResult::NO;

  while (!input_.empty() && boundaryResult != BoundaryResult::PARTIAL) {
    const IOBuf* head = input_.front();
    uint64_t len = head->length();
    const uint8_t* ptr = head->data();

    /* iterate through first character matches */
    while (len > 0 && (ptr = (const uint8_t*)memchr(ptr, boundary_[0], len))) {
      /* calculate length after match */
      uint64_t readlen = (ptr - head->data());
      len = head->length() - readlen;
      boundaryResult =
          isBoundary(*head, readlen, boundary_.data(), boundary_.length());
      if (boundaryResult == BoundaryResult::YES) {
        CHECK(readlen < head->length());
        bool hasCr = false;
        if (readlen == 0 && pendingCR_) {
          pendingCR_.reset();
        }
        if (readlen > 0) {
          // If the last read char is a CR omit from result
          Cursor c(head);
          c.skip(readlen - 1);
          uint8_t ch = c.read<uint8_t>();
          if (ch == '\r') {
            --readlen;
            hasCr = true;
          }
        }
        result.append(std::move(pendingCR_));
        result.append(input_.split(readlen));
        uint32_t trimLen = boundary_.length() + (hasCr ? 1 : 0);
        input_.trimStart(trimLen);
        bytesProcessed_ += readlen + trimLen;
        foundBoundary = true;
        return result;
      } else if (boundaryResult == BoundaryResult::PARTIAL) {
        break;
      } else if (pendingCR_) {
        // not a match, append pending CR to result
        result.append(std::move(pendingCR_));
      }

      /* next character */
      ptr++;
      len--;
    }
    uint64_t resultLen = ptr ? ptr - head->data() : head->length();
    // Put pendingCR_ in result if there was no partial match in head, or a
    // partial match starting after the first character
    if ((boundaryResult == BoundaryResult::NO || resultLen > 0) && pendingCR_) {
      result.append(std::move(pendingCR_));
    }
    // the boundary does not start through resultLen, append it
    // to result, except maybe the last char if it's a CR.
    if (resultLen > 0 && head->data()[resultLen - 1] == '\r') {
      result.append(input_.split(resultLen - 1));
      CHECK(!pendingCR_);
      pendingCR_ = input_.split(1);
    } else {
      result.append(input_.split(resultLen));
    }
    bytesProcessed_ += resultLen;
  }

  // reached the end but no boundary found
  foundBoundary = false;

  return result;
}

void RFC1867Codec::onIngressEOM() {
  if (state_ == ParserState::FIELD_DATA) {
    LOG(WARNING) << "Field not terminated by boundary";
    if (callback_) {
      callback_->onFieldEnd(false, bytesProcessed_);
    }
  }
  if (state_ != ParserState::HEADERS_START && state_ != ParserState::ERROR &&
      state_ != ParserState::DONE) {
    if (callback_) {
      LOG(ERROR) << "onIngressEOM with state_=" << (uint8_t)state_;
      callback_->onError();
    }
  }
  state_ = ParserState::START;
}

} // namespace proxygen

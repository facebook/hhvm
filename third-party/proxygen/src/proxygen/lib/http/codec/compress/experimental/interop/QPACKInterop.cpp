/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/File.h>
#include <folly/FileUtil.h>
#include <folly/init/Init.h>
#include <folly/io/Cursor.h>
#include <folly/portability/GFlags.h>
#include <fstream>
#include <proxygen/lib/http/codec/compress/QPACKCodec.h>
#include <proxygen/lib/http/codec/compress/experimental/simulator/CompressionUtils.h>
#include <proxygen/lib/http/codec/compress/experimental/simulator/SimStreamingCallback.h>
#include <proxygen/lib/http/codec/compress/test/HTTPArchive.h>

using namespace proxygen;
using namespace proxygen::compress;
using namespace folly;
using namespace folly::io;

DEFINE_string(output, "compress.out", "Output file for encoding");
DEFINE_string(input, "compress.in", "Input file for decoding");
DEFINE_string(har, "", "HAR file to compress or compare");
DEFINE_string(mode, "encode", "<encode|decode>");
DEFINE_bool(ack, true, "Encoder assumes immediate ack of all frames");
DEFINE_int32(table_size, 4096, "Dynamic table size");
DEFINE_int32(max_blocking, 100, "Max blocking streams");
DEFINE_bool(public, false, "Public HAR file");

namespace {

void writeFrame(folly::io::QueueAppender& appender,
                uint64_t streamId,
                std::unique_ptr<folly::IOBuf> buf) {
  appender.writeBE<uint64_t>(streamId);
  appender.writeBE<uint32_t>(buf->computeChainDataLength());
  appender.insert(std::move(buf));
}

void encodeBlocks(QPACKCodec& decoder,
                  std::vector<std::vector<compress::Header>>& blocks) {
  uint64_t streamId = 1;
  QPACKCodec encoder;
  encoder.setMaxVulnerable(FLAGS_max_blocking);
  encoder.setEncoderHeaderTableSize(FLAGS_table_size);
  folly::File outputF(FLAGS_output, O_CREAT | O_RDWR | O_TRUNC);
  IOBufQueue outbuf;
  QueueAppender appender(&outbuf, 1000);
  uint64_t bytesIn = 0;
  uint64_t bytesOut = 0;
  for (auto& block : blocks) {
    auto result = encoder.encode(block, streamId);
    // always write stream before control to test decoder blocking
    SimStreamingCallback cb(streamId, nullptr);
    if (result.stream) {
      decoder.decodeStreaming(streamId,
                              result.stream->clone(),
                              result.stream->computeChainDataLength(),
                              &cb);
      writeFrame(appender, streamId, std::move(result.stream));
    }
    if (result.control) {
      decoder.decodeEncoderStream(result.control->clone());
      writeFrame(appender, 0, std::move(result.control));
      if (FLAGS_ack) {
        // There can be ICI when the decoder is non-blocking
        auto res = decoder.encodeInsertCountInc();
        if (res) {
          encoder.decodeDecoderStream(std::move(res));
        }
      }
    }
    if (FLAGS_ack) {
      if (cb.acknowledge) {
        encoder.decodeDecoderStream(decoder.encodeHeaderAck(streamId));
      } else {
        auto res = decoder.encodeInsertCountInc();
        if (res) {
          encoder.decodeDecoderStream(std::move(res));
        }
      }
    }
    bytesIn += encoder.getEncodedSize().uncompressed;
    auto out = outbuf.move();
    auto iov = out->getIov();
    bytesOut += writevFull(outputF.fd(), iov.data(), iov.size());
    streamId++;
  }
  LOG(INFO) << "Encoded " << (streamId - 1) << " streams.  Bytes in=" << bytesIn
            << " Bytes out=" << bytesOut
            << " Ratio=" << int32_t(100 * (1 - (bytesOut / double(bytesIn))));
}

void encodeHar(QPACKCodec& decoder, const proxygen::HTTPArchive& har) {
  std::vector<std::vector<compress::Header>> blocks;
  std::vector<std::vector<std::string>> cookies{har.requests.size()};
  uint32_t i = 0;
  for (auto& req : har.requests) {
    blocks.emplace_back(prepareMessageForCompression(req, cookies[i++]));
  }
  encodeBlocks(decoder, blocks);
}

class Reader {
  std::string filename;

 public:
  explicit Reader(const std::string& fname) : filename(fname) {
  }
  virtual ~Reader() {
  }

  virtual ssize_t read() {
    ssize_t rc = -1;
    folly::IOBufQueue inbuf{folly::IOBufQueue::cacheChainLength()};
    folly::File inputF(filename, O_RDONLY);
    do {
      auto pre = inbuf.preallocate(4096, 4096);
      rc = readNoInt(inputF.fd(), pre.first, pre.second);
      if (rc < 0) {
        LOG(ERROR) << "Read failed on " << FLAGS_input;
        return 1;
      }
      inbuf.postallocate(rc);
      onIngress(inbuf);
    } while (rc != 0);
    if (!inbuf.empty()) {
      LOG(ERROR) << "Premature end of file";
      return 1;
    }

    return rc;
  }

  virtual void onIngress(folly::IOBufQueue& inbuf) = 0;
};

class CompressedReader : public Reader {
  enum { HEADER, DATA } state{HEADER};
  uint64_t streamId{0};
  uint32_t length{0};
  std::function<void(uint64_t, uint32_t, std::unique_ptr<folly::IOBuf>)>
      callback;

 public:
  explicit CompressedReader(
      std::function<void(uint64_t, uint32_t, std::unique_ptr<folly::IOBuf>)> cb)
      : Reader(FLAGS_input), callback(cb) {
  }

  void onIngress(folly::IOBufQueue& inbuf) override {
    while (true) {
      if (state == HEADER) {
        if (inbuf.chainLength() < (sizeof(uint64_t) + sizeof(uint32_t))) {
          return;
        }
        Cursor c(inbuf.front());
        streamId = c.readBE<uint64_t>();
        length = c.readBE<uint32_t>();
        inbuf.trimStart(sizeof(uint64_t) + sizeof(uint32_t));
        state = DATA;
      }
      if (state == DATA) {
        if (inbuf.chainLength() < length) {
          return;
        }
        auto buf = inbuf.split(length);
        callback(streamId, length, std::move(buf));
        state = HEADER;
      }
    }
  }
};

int decodeAndVerify(QPACKCodec& decoder, const proxygen::HTTPArchive& har) {
  std::map<uint64_t, SimStreamingCallback> streams;
  CompressedReader creader([&](uint64_t streamId,
                               uint32_t length,
                               std::unique_ptr<folly::IOBuf> buf) {
    if (streamId == 0) {
      CHECK_EQ(decoder.decodeEncoderStream(std::move(buf)),
               HPACK::DecodeError::NONE);
    } else {
      auto res = streams.emplace(
          std::piecewise_construct,
          std::forward_as_tuple(streamId),
          std::forward_as_tuple(streamId, nullptr, FLAGS_public));
      decoder.decodeStreaming(
          streamId, std::move(buf), length, &res.first->second);
    }
  });
  if (creader.read()) {
    return 1;
  }
  size_t i = 0;
  for (const auto& req : streams) {
    if (req.second.error != HPACK::DecodeError::NONE) {
      LOG(ERROR) << "request=" << req.first
                 << " failed to decode error=" << req.second.error;
      return 1;
    }
    if (!(req.second.msg == har.requests[i])) {
      LOG(ERROR) << "requests are not equal, got=" << req.second.msg
                 << " expected=" << har.requests[i];
    }
    i++;
  }
  LOG(INFO) << "Verified " << i << " streams.";
  return 0;
}

class QIFCallback : public HPACK::StreamingCallback {
 public:
  QIFCallback(uint64_t id_, std::ofstream& of_) : id(id_), of(of_) {
  }

  void onHeader(const HPACKHeaderName& name,
                const folly::fbstring& value) override {
    if (first) {
      of << "# stream " << id << std::endl;
      first = false;
    }
    of << name.get() << "\t" << value << std::endl;
  }
  void onHeadersComplete(HTTPHeaderSize /*decodedSize*/,
                         bool /*acknowledge*/) override {
    of << std::endl;
    complete = true;
  }
  void onDecodeError(HPACK::DecodeError decodeError) override {
    LOG(FATAL) << "Decode error with stream=" << id << " err=" << decodeError;
  }

  uint64_t id{0};
  std::ofstream& of;
  bool first{true};
  bool complete{false};
};

int decodeToQIF(QPACKCodec& decoder) {
  std::ofstream of(FLAGS_output, std::ofstream::trunc);
  std::map<uint64_t, QIFCallback> streams;
  uint64_t encoderStreamBytes = 0;
  CompressedReader creader([&](uint64_t streamId,
                               uint32_t length,
                               std::unique_ptr<folly::IOBuf> buf) {
    if (streamId == 0) {
      CHECK_EQ(decoder.decodeEncoderStream(std::move(buf)),
               HPACK::DecodeError::NONE);
      encoderStreamBytes += length;
    } else {
      auto res = streams.emplace(std::piecewise_construct,
                                 std::forward_as_tuple(streamId),
                                 std::forward_as_tuple(streamId, of));
      decoder.decodeStreaming(
          streamId, std::move(buf), length, &res.first->second);
    }
  });
  if (creader.read()) {
    return 1;
  }

  for (const auto& stream : streams) {
    CHECK(stream.second.complete)
        << "Stream " << stream.first << " didn't complete";
  }
  LOG(INFO) << "encoderStreamBytes=" << encoderStreamBytes;
  return 0;
}

int interopHAR(QPACKCodec& decoder) {
  std::unique_ptr<HTTPArchive> har =
      (FLAGS_public) ? HTTPArchive::fromPublicFile(FLAGS_har)
                     : HTTPArchive::fromFile(FLAGS_har);
  if (!har) {
    LOG(ERROR) << "Failed to read har file='" << FLAGS_har << "'";
    return 1;
  }
  if (FLAGS_mode == "encode") {
    encodeHar(decoder, *har);
  } else if (FLAGS_mode == "decode") {
    return decodeAndVerify(decoder, *har);
  } else {
    LOG(ERROR) << "Usage" << std::endl;
    return 1;
  }
  return 0;
}

struct QIFReader : public Reader {

  std::vector<std::string> strings;
  std::vector<std::vector<Header>> blocks{1};
  enum { LINESTART, COMMENT, NAME, VALUE, EOL } state_{LINESTART};
  bool seenR{false};

  QIFReader() : Reader(FLAGS_input) {
    strings.reserve(32768);
  }

  ssize_t read() override {
    ssize_t rc = Reader::read();
    if (rc != 0) {
      return rc;
    }
    CHECK(blocks.back().empty());
    blocks.pop_back();
    return 0;
  }

  static bool iseol(uint8_t ch) {
    return ch == '\r' || ch == '\n';
  }

  void onIngress(folly::IOBufQueue& input) override {
    Cursor c(input.front());
    while (!c.isAtEnd()) {
      switch (state_) {
        case LINESTART: {
          seenR = false;
          auto p = c.peek();
          switch (p.first[0]) {
            case '#':
              state_ = COMMENT;
              break;
            case '\r':
            case '\n':
              if (!blocks.back().empty()) {
                blocks.emplace_back();
              }
              state_ = EOL;
              break;
            default:
              state_ = NAME;
              strings.emplace_back();
          }
          break;
        }
        case COMMENT:
          c.skipWhile([](uint8_t ch) { return !iseol(ch); });
          if (!c.isAtEnd()) {
            state_ = EOL;
          }
          break;
        case EOL: {
          auto p = c.peek();
          if (p.first[0] == '\n') {
            c.skip(1);
            state_ = LINESTART;
          } else if (seenR) { // \r followed by anything but \n -> mac newline
            state_ = LINESTART;
          } else if (p.first[0] == '\r') {
            c.skip(1);
            seenR = true;
          }
          break;
        }
        case NAME:
          strings.back() += c.readWhile([](uint8_t ch) { return ch != '\t'; });
          if (!c.isAtEnd()) {
            c.skip(1);
            state_ = VALUE;
            strings.emplace_back();
          }
          break;
        case VALUE:
          strings.back() += c.readWhile([](uint8_t ch) { return !iseol(ch); });
          if (!c.isAtEnd()) {
            CHECK_GE(strings.size(), 2);
            blocks.back().emplace_back(compress::Header::makeHeaderForTest(
                *(strings.rbegin() + 1), *strings.rbegin()));
            state_ = EOL;
          }
          break;
      }
    }
    input.move();
  }
};

int interopQIF(QPACKCodec& decoder) {
  if (FLAGS_mode == "encode") {
    QIFReader reader;
    if (reader.read() != 0) {
      LOG(ERROR) << "Failed to read QIF file='" << FLAGS_input << "'";
      return 1;
    }
    encodeBlocks(decoder, reader.blocks);
  } else if (FLAGS_mode == "decode") {
    decodeToQIF(decoder);
  } else {
    LOG(ERROR) << "Usage" << std::endl;
    return 1;
  }

  return 0;
}

} // namespace

int main(int argc, char** argv) {
  folly::init(&argc, &argv, true);
  QPACKCodec decoder;
  decoder.setEncoderHeaderTableSize(FLAGS_table_size);
  std::vector<compress::Header> empty;
  auto res = decoder.encode(empty, 0);
  decoder.setMaxBlocking(FLAGS_max_blocking);
  decoder.setDecoderHeaderTableMaxSize(FLAGS_table_size);
  decoder.decodeEncoderStream(std::move(res.control));
  if (!FLAGS_har.empty()) {
    return interopHAR(decoder);
  } else {
    return interopQIF(decoder);
  }
}

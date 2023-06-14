/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/HPACKEncoder.h>

using std::vector;

namespace proxygen {

std::unique_ptr<folly::IOBuf> HPACKEncoder::encode(
    const vector<HPACKHeader>& headers, uint32_t headroom) {
  if (headroom) {
    streamBuffer_.addHeadroom(headroom);
  }
  handlePendingContextUpdate(streamBuffer_, table_.capacity());
  for (const auto& header : headers) {
    encodeHeader(header.name, header.value); // const, string piece
  }
  return streamBuffer_.release();
}

void HPACKEncoder::encode(const vector<HPACKHeader>& headers,
                          folly::IOBufQueue& writeBuf) {
  streamBuffer_.setWriteBuf(&writeBuf);
  handlePendingContextUpdate(streamBuffer_, table_.capacity());
  for (const auto& header : headers) {
    encodeHeader(header.name, header.value); // const, string piece
  }
  streamBuffer_.setWriteBuf(nullptr);
}

void HPACKEncoder::startEncode(folly::IOBufQueue& writeBuf) {
  streamBuffer_.setWriteBuf(&writeBuf);
  handlePendingContextUpdate(streamBuffer_, table_.capacity());
}

void HPACKEncoder::completeEncode() {
  streamBuffer_.setWriteBuf(nullptr);
}

size_t HPACKEncoder::encodeHeader(HTTPHeaderCode code,
                                  const std::string& value) {
  DCHECK_NE(code, HTTP_HEADER_OTHER);
  HPACKHeaderName name(code);
  size_t uncompressed = name.size() + value.size() + 2;
  encodeHeader(name, value); // const, string piece
  return uncompressed;
}

size_t HPACKEncoder::encodeHeader(HTTPHeaderCode code,
                                  folly::fbstring&& value) {
  DCHECK_NE(code, HTTP_HEADER_OTHER);
  HPACKHeaderName name(code);
  size_t uncompressed = name.size() + value.size() + 2;
  encodeHeader(std::move(name), std::move(value));
  return uncompressed;
}

size_t HPACKEncoder::encodeHeader(const std::string& nameStr,
                                  const std::string& value) {
  HPACKHeaderName name(nameStr);
  size_t uncompressed = name.size() + value.size() + 2;
  encodeHeader(std::move(name), folly::StringPiece(value)); // &&, StringPiece
  return uncompressed;
}

void HPACKEncoder::encodeAsLiteralImpl(const HPACKHeaderName& name,
                                       uint32_t nameIndex,
                                       folly::StringPiece value,
                                       bool& indexing) {
  if (HPACKHeader::bytes(name.size(), value.size()) > table_.capacity()) {
    // May want to investigate further whether or not this is wanted.
    // Flushing the table on a large header frees up some memory,
    // however, there will be no compression due to an empty table, and
    // the table will fill up again fairly quickly
    indexing = false;
  }

  HPACK::Instruction instruction =
      (indexing) ? HPACK::LITERAL_INC_INDEX : HPACK::LITERAL;

  encodeLiteral(name, value, nameIndex, instruction);
}

bool HPACKEncoder::encodeAsLiteral(const HPACKHeaderName& name,
                                   uint32_t nameIndex,
                                   folly::StringPiece value,
                                   bool indexing) {
  encodeAsLiteralImpl(name, nameIndex, value, indexing);
  // indexed ones need to get added to the header table
  if (indexing) {
    CHECK(table_.add(HPACKHeader(name, value)));
  }
  return true;
}

bool HPACKEncoder::encodeAsLiteral(HPACKHeaderName&& name,
                                   uint32_t nameIndex,
                                   folly::fbstring&& value,
                                   bool indexing) {
  encodeAsLiteralImpl(name, nameIndex, value, indexing);
  // indexed ones need to get added to the header table
  if (indexing) {
    CHECK(table_.add(HPACKHeader(std::move(name), std::move(value))));
  }
  return true;
}

bool HPACKEncoder::encodeAsLiteral(HPACKHeaderName&& name,
                                   uint32_t nameIndex,
                                   folly::StringPiece value,
                                   bool indexing) {
  encodeAsLiteralImpl(name, nameIndex, value, indexing);
  // indexed ones need to get added to the header table
  if (indexing) {
    CHECK(table_.add(HPACKHeader(std::move(name), value)));
  }
  return true;
}

void HPACKEncoder::encodeLiteral(const HPACKHeaderName& name,
                                 folly::StringPiece value,
                                 uint32_t nameIndex,
                                 const HPACK::Instruction& instruction) {
  // name
  if (nameIndex) {
    VLOG(10) << "encoding name index=" << nameIndex;
    streamBuffer_.encodeInteger(nameIndex, instruction);
  } else {
    streamBuffer_.encodeInteger(0, instruction);
    streamBuffer_.encodeLiteral(name.get());
  }
  // value
  streamBuffer_.encodeLiteral(value);
}

void HPACKEncoder::encodeAsIndex(uint32_t index) {
  VLOG(10) << "encoding index=" << index;
  streamBuffer_.encodeInteger(index, HPACK::INDEX_REF);
}

folly::Optional<uint32_t> HPACKEncoder::encodeHeaderImpl(
    const HPACKHeaderName& name, folly::StringPiece value, bool& indexable) {
  uint32_t index = 0;
  uint32_t nameIndex = 0;
  // Check to see if the header is the static or dynamic table
  std::tie(index, nameIndex) = getIndex(name, value);

  // Finally encode the header as determined above
  if (index) {
    encodeAsIndex(index);
    return folly::none;
  } else {
    indexable =
        HPACKHeader::bytes(name.size(), value.size()) <= table_.capacity() &&
        (!indexingStrat_ ||
         indexingStrat_->indexHeader(
             name, value, nameIndex > 0 && !isStatic(nameIndex)));

    // caller must encodeAsLiteral
    return nameIndex;
  }
}

void HPACKEncoder::encodeHeader(const HPACKHeaderName& name,
                                folly::StringPiece value) {
  bool indexable = false;
  auto nameIndex = encodeHeaderImpl(name, value, indexable);
  if (nameIndex) {
    encodeAsLiteral(name, *nameIndex, value, indexable);
  }
}

void HPACKEncoder::encodeHeader(HPACKHeaderName&& name,
                                folly::fbstring&& value) {
  bool indexable = false;
  auto nameIndex = encodeHeaderImpl(name, value, indexable);
  if (nameIndex) {
    encodeAsLiteral(std::move(name), *nameIndex, std::move(value), indexable);
  }
}

void HPACKEncoder::encodeHeader(HPACKHeaderName&& name,
                                folly::StringPiece value) {
  bool indexable = false;
  auto nameIndex = encodeHeaderImpl(name, value, indexable);
  if (nameIndex) {
    encodeAsLiteral(std::move(name), *nameIndex, value, indexable);
  }
}

} // namespace proxygen

/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/transport/http2/common/H2Channel.h>

#include <folly/Range.h>
#include <folly/String.h>
#include <proxygen/lib/http/codec/CodecUtil.h>
#include <proxygen/lib/utils/Base64.h>
#include <thrift/lib/cpp/transport/THeader.h>

namespace apache {
namespace thrift {

using proxygen::HTTPMessage;
using std::string;

void H2Channel::encodeHeaders(
    const transport::THeader::StringToStringMap& source,
    HTTPMessage& dest) noexcept {
  auto& msgHeaders = dest.getHeaders();
  for (auto it = source.begin(); it != source.end(); ++it) {
    // Keys in Thrift may not comply with HTTP key format requirements.
    // The following algorithm is implemented at multiple locations and
    // should be kept in sync.
    //
    // If the key does not contain a ":", we assume it is compliant
    // and we use the key and value as is.  In case such a key is not
    // compliant, things will break.
    //
    // If it contains a ":", we encode both the key and the value.  We
    // add a prefix tag ("encode_") to the and encode both the key and
    // the value into the value.
    std::string headerNameLowercase(it->first);
    folly::toLowerAscii(headerNameLowercase);
    if (!proxygen::CodecUtil::validateHeaderName(
            folly::ByteRange(folly::StringPiece(headerNameLowercase)),
            proxygen::CodecUtil::HEADER_NAME_STRICT) ||
        !proxygen::CodecUtil::validateHeaderValue(
            folly::ByteRange(folly::StringPiece(it->second)),
            proxygen::CodecUtil::CtlEscapeMode::STRICT)) {
      auto name = proxygen::Base64::urlEncode(folly::StringPiece(it->first));
      auto value = proxygen::Base64::urlEncode(folly::StringPiece(it->second));
      msgHeaders.set(
          folly::to<std::string>("encode_", name),
          folly::to<std::string>(name, "_", value));
    } else {
      msgHeaders.set(it->first, it->second);
    }
  }
}

void H2Channel::decodeHeaders(
    const HTTPMessage& source,
    transport::THeader::StringToStringMap& dest,
    RequestRpcMetadata* metadata) noexcept {
  auto decodeAndCopyKeyValue = [&](proxygen::HTTPHeaderCode code,
                                   const string& key,
                                   const string& val) {
    if (code == proxygen::HTTP_HEADER_HOST ||
        code == proxygen::HTTP_HEADER_USER_AGENT ||
        code == proxygen::HTTP_HEADER_CONTENT_TYPE) {
      // These headers are not part of thrift metadata
      return;
    }

    if (code == proxygen::HTTP_HEADER_OTHER) {
      // This decodes key-value pairs that have been encoded using
      // encodeHeaders() or equivalent methods.  If the key starts with
      // "encode_", the value is split at the underscore and then the
      // key and value are decoded from there.  The key is not used
      // because it will get converted to lowercase and therefore the
      // original key cannot be recovered.
      if (folly::StringPiece(key).startsWith("encode_")) {
        auto us = val.find("_");
        if (us != string::npos) {
          // TODO: urlDecode with StringPiece?
          auto decodedKey = proxygen::Base64::urlDecode(val.substr(0, us));
          auto decodedVal = proxygen::Base64::urlDecode(val.substr(us + 1));
          dest.emplace(std::move(decodedKey), std::move(decodedVal));
          return;
        }
        LOG(ERROR) << "Encoded value does not contain '_'; preserving original";
      }

      // If request metadata present, handle special thrift headers
      if (metadata && handleThriftMetadata(metadata, key, val)) {
        return;
      }
    }
    dest.emplace(key, val);
  };
  source.getHeaders().forEachWithCode(decodeAndCopyKeyValue);
}

bool H2Channel::handleThriftMetadata(
    RequestRpcMetadata* metadata,
    const std::string& key,
    const std::string& value) noexcept {
  if (key == transport::THeader::CLIENT_TIMEOUT_HEADER) {
    auto parsed = folly::tryTo<int64_t>(value);
    if (!parsed) {
      LOG(INFO) << "Bad client timeout " << value;
      return false;
    }
    metadata->clientTimeoutMs_ref() = *parsed;
    return true;
  }
  if (key == RPC_KIND) {
    auto parsed = folly::tryTo<int32_t>(value);
    if (!parsed) {
      LOG(INFO) << "Bad Request Kind " << value;
      return false;
    }
    metadata->kind_ref() = static_cast<RpcKind>(*parsed);
    return true;
  }
  if (key == transport::THeader::QUEUE_TIMEOUT_HEADER) {
    auto parsed = folly::tryTo<int64_t>(value);
    if (!parsed) {
      LOG(INFO) << "Bad client timeout " << value;
      return false;
    }
    metadata->queueTimeoutMs_ref() = *parsed;
    return true;
  }
  if (key == transport::THeader::PRIORITY_HEADER) {
    auto parsed = folly::tryTo<int32_t>(value);
    if (!parsed) {
      LOG(INFO) << "Bad method priority " << value;
      return false;
    }
    auto pr = static_cast<RpcPriority>(*parsed);
    if (pr >= RpcPriority::N_PRIORITIES) {
      LOG(INFO) << "Too large value for method priority " << value;
      return false;
    }
    metadata->priority_ref() = pr;
    return true;
  }
  return false;
}

} // namespace thrift
} // namespace apache

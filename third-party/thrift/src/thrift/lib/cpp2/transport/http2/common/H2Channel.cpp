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
#include <folly/base64.h>
#include <proxygen/lib/http/codec/CodecUtil.h>
#include <thrift/lib/cpp/transport/THeader.h>

namespace {
template <uint64_t x>
// TODO: Replace `encode_<base64url(name)>` with encode_thrift_header, and add
// that header to proxygen/lib/http/HTTPCommonHeaders.txt
// encode_C2VYDMLJZXJVDXRLCJPOB3BFCGF0AA = "servicerouter:hop_path"
typename std::enable_if<(x > 200), bool>::type isEncodeHeader(
    proxygen::HTTPHeaderCode code, const std::string& key) {
  return code == proxygen::HTTP_HEADER_ENCODE_C2VYDMLJZXJVDXRLCJPOB3BFCGF0AA ||
      (code == proxygen::HTTP_HEADER_OTHER &&
       folly::StringPiece(key).startsWith("encode_"));
}
template <uint64_t x>
typename std::enable_if<(x <= 200), bool>::type isEncodeHeader(
    proxygen::HTTPHeaderCode code, const std::string& key) {
  return (
      code == proxygen::HTTP_HEADER_OTHER &&
      folly::StringPiece(key).startsWith("encode_"));
}
} // namespace

namespace apache::thrift {

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
      auto name = folly::base64URLEncode(it->first);
      auto value = folly::base64URLEncode(it->second);
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
    if (metadata && handleThriftMetadata(metadata, code, key, val)) {
      return;
    }
    if (isEncodeHeader<proxygen::HTTPCommonHeaders::num_codes>(code, key)) {
      // This decodes key-value pairs that have been encoded using
      // encodeHeaders() or equivalent methods.  If the key starts with
      // "encode_", the value is split at the underscore and then the
      // key and value are decoded from there.  The key is not used
      // because it will get converted to lowercase and therefore the
      // original key cannot be recovered.
      auto us = val.find('_');
      if (us != string::npos) {
        std::string decodedKey;
        std::string decodedVal;
        try {
          decodedKey = folly::base64URLDecode(std::string_view(val.data(), us));
          decodedVal = folly::base64URLDecode(
              std::string_view(val.data() + us + 1, val.length() - us - 1));
        } catch (...) {
          // Code previously used proxygen::Base64::urlDecode which converted
          // exceptions to empty string.  Preserving that behavior.
        }
        dest.emplace(std::move(decodedKey), std::move(decodedVal));
        return;
      }
      LOG(ERROR) << "Encoded value does not contain '_'; preserving original";
    } else if (code != proxygen::HTTP_HEADER_OTHER) {
      DCHECK(!folly::StringPiece(key).startsWith("encode_"));
    }
    dest.emplace(key, val);
  };
  source.getHeaders().forEachWithCode(decodeAndCopyKeyValue);
}

bool H2Channel::handleThriftMetadata(
    RequestRpcMetadata* metadata,
    proxygen::HTTPHeaderCode code,
    const std::string&,
    const std::string& value) noexcept {
  if (code == proxygen::HTTP_HEADER_RPCKIND) {
    auto parsed = folly::tryTo<int32_t>(value);
    if (!parsed) {
      LOG(INFO) << "Bad Request Kind " << value;
      return false;
    }
    metadata->kind() = static_cast<RpcKind>(*parsed);
    return true;
  } else if (code == proxygen::HTTP_HEADER_QUEUE_TIMEOUT) {
    auto parsed = folly::tryTo<int64_t>(value);
    if (!parsed) {
      LOG(INFO) << "Bad client timeout " << value;
      return false;
    }
    metadata->queueTimeoutMs() = *parsed;
    return true;
  } else if (code == proxygen::HTTP_HEADER_THRIFT_PRIORITY) {
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
    metadata->priority() = pr;
    return true;
  } else if (code == proxygen::HTTP_HEADER_CLIENT_TIMEOUT) {
    auto parsed = folly::tryTo<int64_t>(value);
    if (!parsed) {
      LOG(INFO) << "Bad client timeout " << value;
      return false;
    }
    metadata->clientTimeoutMs() = *parsed;
    return true;
  }
  return false;
}

} // namespace apache::thrift

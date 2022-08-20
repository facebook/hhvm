/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/test/HTTPArchive.h>

#include <algorithm>
#include <folly/io/IOBuf.h>
#include <folly/json.h>
#include <fstream>
#include <glog/logging.h>
#include <ios>
#include <string>

using folly::IOBuf;
using std::ifstream;
using std::ios;
using std::string;
using std::unique_ptr;
using std::vector;

namespace {

folly::Optional<std::chrono::steady_clock::time_point> parseHTTPArchiveTime(
    const std::string& s) {
  struct tm tm = {};

  if (s.empty()) {
    return folly::none;
  }

  uint32_t ms = 0;
  // Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
  // Example: 2013-12-09T16:38:03.701Z
  if (sscanf(s.c_str(),
             "%d-%d-%dT%d:%d:%d.%dZ",
             &tm.tm_year,
             &tm.tm_mon,
             &tm.tm_mday,
             &tm.tm_hour,
             &tm.tm_min,
             &tm.tm_sec,
             &ms) != 7) {
    return folly::none;
  }
  // Per the spec, for some reason the API is inconsistent and requires
  // years to be offset from 1900 and even more strange for month to be 0
  // offset despite the fact it expects days to be 1 offset
  // https://linux.die.net/man/3/mktime
  tm.tm_year = tm.tm_year - 1900;
  tm.tm_mon = tm.tm_mon - 1;

  auto res = mktime(&tm);
  return std::chrono::steady_clock::time_point(std::chrono::seconds(res) +
                                               std::chrono::milliseconds(ms));
}

proxygen::HTTPMessage extractMessage(folly::dynamic& obj,
                                     const std::string& timeStr,
                                     bool request) {
  proxygen::HTTPMessage msg;
  auto& headersObj = obj["headers"];
  for (size_t i = 0; i < headersObj.size(); i++) {
    string name = headersObj[i]["name"].asString();
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    if (name[0] == ':') {
      if (name == ":host" || name == ":authority") {
        name = "host";
      } else {
        continue;
      }
    }
    msg.getHeaders().add(name, headersObj[i]["value"].asString());
  }
  try {
    if (request) {
      msg.setURL(obj["url"].asString());
      msg.setMethod(obj["method"].asString());
      auto t = parseHTTPArchiveTime(timeStr);
      if (t.has_value()) {
        msg.setStartTime(t.value());
      }
    } else {
      msg.setStatusCode(obj["status"].asInt());
      msg.setStatusMessage(obj["statusText"].asString());
    }

    string proto;
    string version;
    folly::split('/', obj["httpVersion"].asString(), proto, version);
    msg.setVersionString(version);
  } catch (...) {
  }

  return msg;
}

proxygen::HTTPMessage extractMessageFromPublic(folly::dynamic& obj) {
  proxygen::HTTPMessage msg;
  auto& headersObj = obj["headers"];
  for (size_t i = 0; i < headersObj.size(); i++) {
    auto& headerObj = headersObj[i];
    for (auto& k : headerObj.keys()) {
      string name = k.asString();
      string value = headerObj[name].asString();
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      msg.getHeaders().add(name, value);
    }
  }
  return msg;
}
} // namespace

namespace proxygen {

std::unique_ptr<IOBuf> readFileToIOBuf(const std::string& filename) {
  // read the contents of the file
  ifstream file(filename);
  if (!file.is_open()) {
    LOG(ERROR) << "could not open file '" << filename << "'";
    return nullptr;
  }
  file.seekg(0, ios::end);
  int64_t size = file.tellg();
  if (size < 0) {
    LOG(ERROR) << "failed to fetch the position at the end of the file";
    return nullptr;
  }
  file.seekg(0, ios::beg);
  unique_ptr<IOBuf> buffer = IOBuf::create(size + 1);
  file.read((char*)buffer->writableData(), size);
  buffer->writableData()[size] = 0;
  buffer->append(size + 1);
  if (!file) {
    LOG(ERROR) << "error occurred, was able to read only " << file.gcount()
               << " bytes out of " << size;
    return nullptr;
  }
  return buffer;
}

unique_ptr<HTTPArchive> HTTPArchive::fromFile(const string& filename) {
  unique_ptr<HTTPArchive> har = std::make_unique<HTTPArchive>();
  auto buffer = readFileToIOBuf(filename);
  if (!buffer) {
    return nullptr;
  }
  folly::dynamic jsonObj = folly::parseJson((const char*)buffer->data());
  auto entries = jsonObj["log"]["entries"];
  // go over all the transactions
  for (size_t i = 0; i < entries.size(); i++) {
    HTTPMessage msg = extractMessage(
        entries[i]["request"], entries[i]["startedDateTime"].asString(), true);
    if (msg.getHeaders().size() != 0) {
      har->requests.emplace_back(std::move(msg));
    }
    msg = extractMessage(entries[i]["response"], "", false);
    if (msg.getHeaders().size() != 0) {
      har->responses.emplace_back(std::move(msg));
    }
  }

  return har;
}

uint32_t HTTPArchive::getSize(const HTTPMessage& msg) {
  uint32_t size = 0;

  msg.getHeaders().forEach([&size](const string& name, const string& value) {
    size += name.size() + value.size() + 2;
  });
  return size;
}

uint32_t HTTPArchive::getSize(const vector<HPACKHeader>& headers) {
  uint32_t size = 0;

  for (const auto& header : headers) {
    size += header.name.size() + header.value.size() + 2;
  }
  return size;
}

unique_ptr<HTTPArchive> HTTPArchive::fromPublicFile(const string& filename) {
  unique_ptr<HTTPArchive> har = std::make_unique<HTTPArchive>();
  auto buffer = readFileToIOBuf(filename);
  if (!buffer) {
    return nullptr;
  }
  folly::dynamic jsonObj = folly::parseJson((const char*)buffer->data());
  auto entries = jsonObj["cases"];
  // go over all the transactions
  for (size_t i = 0; i < entries.size(); i++) {
    HTTPMessage msg = extractMessageFromPublic(entries[i]);
    if (msg.getHeaders().size() != 0) {
      har->requests.emplace_back(msg);
    }
  }

  return har;
}

std::vector<std::vector<HPACKHeader>> HTTPArchive::convertToHPACK(
    const std::vector<HTTPMessage>& msgs) {
  std::vector<std::vector<HPACKHeader>> result;
  for (const HTTPMessage& msg : msgs) {
    std::vector<HPACKHeader> headers;
    msg.getHeaders().forEach(
        [&headers](const string& name, const string& value) {
          headers.emplace_back(name, value);
        });
    result.emplace_back(std::move(headers));
  }
  return result;
}

} // namespace proxygen

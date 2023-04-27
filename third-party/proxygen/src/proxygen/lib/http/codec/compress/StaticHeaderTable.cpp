/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/StaticHeaderTable.h>

#include <folly/Indestructible.h>

#include <glog/logging.h>
#include <list>

using std::list;

namespace {

// Array of static header table entires pair
// Note: if updating this table (should never have to but whatever), update
// isHeaderNameInTableWithNonEmptyValue as well
const char* s_tableEntries[][2] = {{":authority", ""},
                                   {":method", "GET"},
                                   {":method", "POST"},
                                   {":path", "/"},
                                   {":path", "/index.html"},
                                   {":scheme", "http"},
                                   {":scheme", "https"},
                                   {":status", "200"},
                                   {":status", "204"},
                                   {":status", "206"},
                                   {":status", "304"},
                                   {":status", "400"},
                                   {":status", "404"},
                                   {":status", "500"},
                                   {"accept-charset", ""},
                                   {"accept-encoding", "gzip, deflate"},
                                   {"accept-language", ""},
                                   {"accept-ranges", ""},
                                   {"accept", ""},
                                   {"access-control-allow-origin", ""},
                                   {"age", ""},
                                   {"allow", ""},
                                   {"authorization", ""},
                                   {"cache-control", ""},
                                   {"content-disposition", ""},
                                   {"content-encoding", ""},
                                   {"content-language", ""},
                                   {"content-length", ""},
                                   {"content-location", ""},
                                   {"content-range", ""},
                                   {"content-type", ""},
                                   {"cookie", ""},
                                   {"date", ""},
                                   {"etag", ""},
                                   {"expect", ""},
                                   {"expires", ""},
                                   {"from", ""},
                                   {"host", ""},
                                   {"if-match", ""},
                                   {"if-modified-since", ""},
                                   {"if-none-match", ""},
                                   {"if-range", ""},
                                   {"if-unmodified-since", ""},
                                   {"last-modified", ""},
                                   {"link", ""},
                                   {"location", ""},
                                   {"max-forwards", ""},
                                   {"proxy-authenticate", ""},
                                   {"proxy-authorization", ""},
                                   {"range", ""},
                                   {"referer", ""},
                                   {"refresh", ""},
                                   {"retry-after", ""},
                                   {"server", ""},
                                   {"set-cookie", ""},
                                   {"strict-transport-security", ""},
                                   {"transfer-encoding", ""},
                                   {"user-agent", ""},
                                   {"vary", ""},
                                   {"via", ""},
                                   {"www-authenticate", ""}};

const int kEntriesSize = sizeof(s_tableEntries) / (2 * sizeof(const char*));
} // namespace

namespace proxygen {

bool StaticHeaderTable::isHeaderCodeInTableWithNonEmptyValue(
    HTTPHeaderCode headerCode) {
  switch (headerCode) {
    case HTTP_HEADER_COLON_METHOD:
    case HTTP_HEADER_COLON_PATH:
    case HTTP_HEADER_COLON_SCHEME:
    case HTTP_HEADER_COLON_STATUS:
    case HTTP_HEADER_ACCEPT_ENCODING:
      return true;

    default:
      return false;
  }
}

StaticHeaderTable::StaticHeaderTable(const char* entries[][2], int size)
    : HeaderTable(0) {
  // calculate the size
  list<HPACKHeader> hlist;
  uint32_t byteCount = 0;
  for (int i = 0; i < size; ++i) {
    hlist.push_back(HPACKHeader(entries[i][0], entries[i][1]));
    byteCount += hlist.back().bytes();
  }
  // initialize with a capacity that will exactly fit the static headers
  init(byteCount);
  hlist.reverse();
  for (auto& header : hlist) {
    CHECK(add(std::move(header)));
  }
}

const StaticHeaderTable& StaticHeaderTable::get() {
  static const folly::Indestructible<StaticHeaderTable> table(s_tableEntries,
                                                              kEntriesSize);
  return *table;
}

} // namespace proxygen

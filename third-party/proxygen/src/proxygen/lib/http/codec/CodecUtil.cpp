/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/CodecUtil.h>

#include <folly/String.h>
#include <folly/ThreadLocal.h>
#include <proxygen/lib/http/HeaderConstants.h>
#include <proxygen/lib/http/RFC2616.h>

namespace proxygen {

/**
 * RFC 7230:
 *  token = 1*tchar
 *  tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /
 *          "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA
 *
 * Earlier versions of this table allowed ' ', '"', '/' and '}'
 */
// clang-format off
const char CodecUtil::http_tokens[256] = {
/*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si   */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
        1,      '!',      1,     '#',     '$',     '%',     '&',    '\'',
/*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
        0,       0,      '*',     '+',      0,      '-',     '.',      1,
/*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
       '0',     '1',     '2',     '3',     '4',     '5',     '6',     '7',
/*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
       '8',     '9',      0,       0,       0,       0,       0,       0,
/*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
        0,      'a',     'b',     'c',     'd',     'e',     'f',     'g',
/*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
       'h',     'i',     'j',     'k',     'l',     'm',     'n',     'o',
/*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
       'p',     'q',     'r',     's',     't',     'u',     'v',     'w',
/*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
       'x',     'y',     'z',      0,       0,       0,      '^',     '_',
/*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
       '`',     'a',     'b',     'c',     'd',     'e',     'f',     'g',
/* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
       'h',     'i',     'j',     'k',     'l',     'm',     'n',     'o',
/* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
       'p',     'q',     'r',     's',     't',     'u',     'v',     'w',
/* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
       'x',     'y',     'z',      0,      '|',      1,     '~',       0
};
// clang-format on

bool CodecUtil::hasGzipAndDeflate(const std::string& value,
                                  bool& hasGzip,
                                  bool& hasDeflate) {
  RFC2616::TokenPairVec output;
  output.reserve(RFC2616::kTokenPairVecDefaultSize);
  hasGzip = false;
  hasDeflate = false;
  RFC2616::parseQvalues(value, output);
  for (const auto& encodingQ : output) {
    std::string lower(encodingQ.first.str());
    folly::toLowerAscii(lower);
    // RFC says 3 sig figs
    if (lower == "gzip" && encodingQ.second >= 0.001) {
      hasGzip = true;
    } else if (lower == "deflate" && encodingQ.second >= 0.001) {
      hasDeflate = true;
    }
  }
  return hasGzip && hasDeflate;
}

bool CodecUtil::appendHeaders(const HTTPHeaders& inputHeaders,
                              std::vector<compress::Header>& headers,
                              HTTPHeaderCode headerToCheck) {
  bool headerToCheckExists = false;
  // Add the HTTP headers supplied by the caller, but skip
  // any per-hop headers that aren't supported in HTTP/2.
  inputHeaders.forEachWithCode([&](HTTPHeaderCode code,
                                   const std::string& name,
                                   const std::string& value) {
    static const std::bitset<256> s_perHopHeaderCodes{[] {
      std::bitset<256> bs;
      // HTTP/1.x per-hop headers that have no meaning in HTTP/2
      bs[HTTP_HEADER_CONNECTION] = true;
      bs[HTTP_HEADER_HOST] = true;
      bs[HTTP_HEADER_KEEP_ALIVE] = true;
      bs[HTTP_HEADER_PROXY_CONNECTION] = true;
      bs[HTTP_HEADER_TRANSFER_ENCODING] = true;
      bs[HTTP_HEADER_UPGRADE] = true;
      bs[HTTP_HEADER_SEC_WEBSOCKET_KEY] = true;
      bs[HTTP_HEADER_SEC_WEBSOCKET_ACCEPT] = true;
      return bs;
    }()};

    if (s_perHopHeaderCodes[code] || name.size() == 0 || name[0] == ':') {
      DCHECK_GT(name.size(), 0) << "Empty header";
      DCHECK_NE(name[0], ':') << "Invalid header=" << name;
      return;
    }
    // Note this code will not drop headers named by Connection.  That's the
    // caller's job

    // see HTTP/2 spec, 8.1.2
    DCHECK(name != "TE" || value == "trailers");
    if ((name.size() > 0 && name[0] != ':') && code != HTTP_HEADER_HOST) {
      headers.emplace_back(code, name, value);
    }
    if (code == headerToCheck) {
      headerToCheckExists = true;
    }
  });

  return headerToCheckExists;
}

const std::bitset<256>& CodecUtil::perHopHeaderCodes() {
  static const std::bitset<256> s_perHopHeaderCodes{[] {
    std::bitset<256> bs;
    // HTTP/1.x per-hop headers that have no meaning in HTTP/2
    bs[HTTP_HEADER_CONNECTION] = true;
    bs[HTTP_HEADER_HOST] = true;
    bs[HTTP_HEADER_KEEP_ALIVE] = true;
    bs[HTTP_HEADER_PROXY_CONNECTION] = true;
    bs[HTTP_HEADER_TRANSFER_ENCODING] = true;
    bs[HTTP_HEADER_UPGRADE] = true;
    bs[HTTP_HEADER_SEC_WEBSOCKET_KEY] = true;
    bs[HTTP_HEADER_SEC_WEBSOCKET_ACCEPT] = true;
    return bs;
  }()};
  return s_perHopHeaderCodes;
}
} // namespace proxygen

/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>

namespace proxygen {

/*
 * Struct to hold the encoder and decoder information
 */
struct CompressionInfoPart {
  uint32_t headerTableSize_{0};
  uint32_t bytesStored_{0};
  uint32_t headersStored_{0};
  uint32_t inserts_{0};
  uint32_t blockedInserts_{0};
  uint32_t duplications_{0};
  uint32_t staticRefs_{0};

  CompressionInfoPart(uint32_t headerTableSize,
                      uint32_t bytesStored,
                      uint32_t headersStored,
                      uint32_t inserts,
                      uint32_t blockedInserts,
                      uint32_t duplications,
                      uint32_t staticRefs)
      : headerTableSize_(headerTableSize),
        bytesStored_(bytesStored),
        headersStored_(headersStored),
        inserts_(inserts),
        blockedInserts_(blockedInserts),
        duplications_(duplications),
        staticRefs_(staticRefs) {
  }

  CompressionInfoPart() {
  }

  // copy constructor
  CompressionInfoPart(const CompressionInfoPart&) = default;
  // copy assignment operator
  CompressionInfoPart& operator=(const CompressionInfoPart& other) {
    headerTableSize_ = other.headerTableSize_;
    bytesStored_ = other.bytesStored_;
    headersStored_ = other.headersStored_;
    inserts_ = other.inserts_;
    blockedInserts_ = other.blockedInserts_;
    duplications_ = other.duplications_;
    staticRefs_ = other.staticRefs_;
    return *this;
  }
  // move constructor
  CompressionInfoPart(CompressionInfoPart&&) = default;
  // move assignment operator
  CompressionInfoPart& operator=(CompressionInfoPart&&) = default;
  // destructor
  ~CompressionInfoPart() = default;
};

struct CompressionInfo {
  // Egress table info (encoder)
  CompressionInfoPart egress;

  // Ingress table info (decoder)
  CompressionInfoPart ingress;

  CompressionInfo(uint32_t egressHeaderTableSize,
                  uint32_t egressBytesStored,
                  uint32_t egressHeadersStored,
                  uint32_t egressInserts,
                  uint32_t egressBlockedInserts,
                  uint32_t egressDuplications,
                  uint32_t egressStaticRefs,
                  uint32_t ingressHeaderTableSize,
                  uint32_t ingressBytesStored,
                  uint32_t ingressHeadersStored,
                  uint32_t ingressInserts,
                  uint32_t ingressBlockedInserts,
                  uint32_t ingressDuplications,
                  uint32_t ingressStaticRefs)
      : egress(egressHeaderTableSize,
               egressBytesStored,
               egressHeadersStored,
               egressInserts,
               egressBlockedInserts,
               egressDuplications,
               egressStaticRefs),
        ingress(ingressHeaderTableSize,
                ingressBytesStored,
                ingressHeadersStored,
                ingressInserts,
                ingressBlockedInserts,
                ingressDuplications,
                ingressStaticRefs) {
  }

  CompressionInfo() {
  }

  bool operator==(const CompressionInfo& tableInfo) const {
    return egress.headerTableSize_ == tableInfo.egress.headerTableSize_ &&
           egress.bytesStored_ == tableInfo.egress.bytesStored_ &&
           egress.headersStored_ == tableInfo.egress.headersStored_ &&
           ingress.headerTableSize_ == tableInfo.ingress.headerTableSize_ &&
           ingress.bytesStored_ == tableInfo.ingress.bytesStored_ &&
           ingress.headersStored_ == tableInfo.ingress.headersStored_;
  }
};
} // namespace proxygen

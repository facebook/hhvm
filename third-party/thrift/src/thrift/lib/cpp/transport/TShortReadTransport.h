/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#ifndef _THRIFT_TRANSPORT_TSHORTREADTRANSPORT_H_
#define _THRIFT_TRANSPORT_TSHORTREADTRANSPORT_H_ 1

#include <cstdlib>

#include <thrift/lib/cpp/transport/TTransport.h>
#include <thrift/lib/cpp/transport/TVirtualTransport.h>

namespace apache {
namespace thrift {
namespace transport {
namespace test {

/**
 * This class is only meant for testing.  It wraps another transport.
 * Calls to read are passed through with some probability.  Otherwise,
 * the read amount is randomly reduced before being passed through.
 *
 */
class TShortReadTransport : public TVirtualTransport<TShortReadTransport> {
 public:
  TShortReadTransport(std::shared_ptr<TTransport> transport, double full_prob)
      : transport_(transport), fullProb_(full_prob) {}

  bool isOpen() override { return transport_->isOpen(); }

  bool peek() override { return transport_->peek(); }

  void open() override { transport_->open(); }

  void close() override { transport_->close(); }

  uint32_t read(uint8_t* buf, uint32_t len) {
    if (len == 0) {
      return 0;
    }

    if (rand() / (double)RAND_MAX >= fullProb_) {
      len = 1 + rand() % len;
    }
    return transport_->read(buf, len);
  }

  void write(const uint8_t* buf, uint32_t len) { transport_->write(buf, len); }

  void flush() override { transport_->flush(); }

  const uint8_t* borrow(uint8_t* buf, uint32_t* len) {
    return transport_->borrow(buf, len);
  }

  void consume(uint32_t len) { return transport_->consume(len); }

  std::shared_ptr<TTransport> getUnderlyingTransport() { return transport_; }

 protected:
  std::shared_ptr<TTransport> transport_;
  double fullProb_;
};

} // namespace test
} // namespace transport
} // namespace thrift
} // namespace apache

#endif // #ifndef _THRIFT_TRANSPORT_TSHORTREADTRANSPORT_H_

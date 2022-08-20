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

#ifndef THRIFT_TRANSPORT_TNULLTRANSPORT_H_
#define THRIFT_TRANSPORT_TNULLTRANSPORT_H_ 1

#include <stdint.h>

#include <thrift/lib/cpp/transport/TVirtualTransport.h>

namespace apache {
namespace thrift {
namespace transport {

/**
 * The null transport is a dummy transport that doesn't actually do anything.
 * It's sort of an analogy to /dev/null, you can never read anything from it
 * and it will let you write anything you want to it, though it won't actually
 * go anywhere.
 *
 */
class TNullTransport : public TVirtualTransport<TNullTransport> {
 public:
  TNullTransport() {}

  ~TNullTransport() override {}

  bool isOpen() override { return true; }

  void open() override {}

  void write(const uint8_t* /* buf */, uint32_t /* len */) { return; }
};

} // namespace transport
} // namespace thrift
} // namespace apache

#endif // #ifndef _THRIFT_TRANSPORT_TNULLTRANSPORT_H_

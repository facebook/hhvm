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

#ifndef _THRIFT_TRANSPORT_THTTPSERVER_H_
#define _THRIFT_TRANSPORT_THTTPSERVER_H_ 1

#include <thrift/lib/cpp/transport/THttpTransport.h>

namespace apache {
namespace thrift {
namespace transport {

class THttpServer : public THttpTransport {
 public:
  explicit THttpServer(std::shared_ptr<TTransport> transport);

  ~THttpServer() override;

  void flush() override;

 protected:
  void readHeaders();
  void parseHeader(char* header) override;
  bool parseStatusLine(char* status) override;
  std::string getTimeRFC1123();
};

/**
 * Wraps a transport into HTTP protocol
 */
class THttpServerTransportFactory : public TTransportFactory {
 public:
  THttpServerTransportFactory() {}

  ~THttpServerTransportFactory() override {}

  /**
   * Wraps the transport into a buffered one.
   */
  std::shared_ptr<TTransport> getTransport(
      std::shared_ptr<TTransport> trans) override {
    return std::shared_ptr<TTransport>(new THttpServer(trans));
  }
};

} // namespace transport
} // namespace thrift
} // namespace apache

#endif // #ifndef _THRIFT_TRANSPORT_THTTPSERVER_H_

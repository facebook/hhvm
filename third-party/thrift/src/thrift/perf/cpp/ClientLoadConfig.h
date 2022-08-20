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

#ifndef THRIFT_TEST_PERF_CLIENTLOADCONFIG_H_
#define THRIFT_TEST_PERF_CLIENTLOADCONFIG_H_ 1

#include <folly/SocketAddress.h>
#include <thrift/lib/cpp/test/loadgen/WeightedLoadConfig.h>

namespace apache {
namespace thrift {

namespace test {

class ClientLoadConfig : public loadgen::WeightedLoadConfig {
 public:
  enum OperationEnum {
    OP_NOOP = 0,
    OP_ONEWAY_NOOP,
    OP_ASYNC_NOOP,
    OP_SLEEP,
    OP_ONEWAY_SLEEP,
    OP_BURN,
    OP_ONEWAY_BURN,
    OP_BAD_SLEEP,
    OP_BAD_BURN,
    OP_THROW_ERROR,
    OP_THROW_UNEXPECTED,
    OP_ONEWAY_THROW,
    OP_SEND,
    OP_ONEWAY_SEND,
    OP_RECV,
    OP_SENDRECV,
    OP_ECHO,
    OP_ADD,
    OP_LARGE_CONTAINER,
    OP_ITER_ALL_FIELDS,
    NUM_OPS,
  };

  ClientLoadConfig();

  uint32_t pickOpsPerConnection() override;
  uint32_t getNumWorkerThreads() const override;
  uint64_t getDesiredQPS() const override;

  virtual uint32_t getAsyncClients() const;
  virtual uint32_t getAsyncOpsPerClient() const;
  /**
   * Pick a number of microseconds to use for a sleep operation.
   */
  uint32_t pickSleepUsec();

  /**
   * Pick a number of microseconds to use for a burn operation.
   */
  uint32_t pickBurnUsec();

  /**
   * Pick a number of bytes for a send request
   */
  uint32_t pickSendSize();

  /**
   * Pick a number of bytes for a receive request
   */
  uint32_t pickRecvSize();

  /**
   * Pick a number of elements for a container request
   */
  uint32_t pickContainerSize();

  /**
   * Pick a number of elements for a field in a big struct
   */
  uint32_t pickStructFieldSize();

  /**
   * Make a big struct with 100 string fields
   */
  template <typename T>
  void makeBigStruct(T& bigstruct) {
    *bigstruct.stringField_ref() =
        std::string(this->pickStructFieldSize(), 'a');
    for (int i = 0; i < 100; i++) {
      bigstruct.stringList_ref()->push_back(
          std::string(this->pickStructFieldSize(), 'a'));
    }
  }

  /**
   * Make a large container with several bigstruct objects
   */
  template <typename T>
  void makeBigContainer(std::vector<T>& items) {
    for (auto i = 0u; i < this->pickContainerSize(); i++) {
      T item;
      this->makeBigStruct(item);
      items.push_back(std::move(item));
    }
  }

  const folly::SocketAddress* getAddress() const { return &address_; }

  std::string getAddressHostname() const { return addressHostname_; }

  bool useFramedTransport() const;

  bool useHeaderProtocol() const;

  bool useAsync() const;

  bool useSSL() const;

  bool useSSLTFO() const;

  bool useSingleHost() const;

  bool zlib() const;

  std::string server() const;
  uint32_t port() const;
  std::string key() const;
  std::string cert() const;
  std::string trustedCAList() const;
  std::string ciphers() const;
  bool useTickets() const;

 private:
  uint32_t pickLogNormal(double mean, double sigma);

  folly::SocketAddress address_;
  std::string addressHostname_;
};

} // namespace test
} // namespace thrift
} // namespace apache

#endif // THRIFT_TEST_PERF_CLIENTLOADCONFIG_H_

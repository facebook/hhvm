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

#define __STDC_FORMAT_MACROS

#include <thrift/perf/cpp/ClientWorker2.h>

#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp/async/TAsyncSSLSocket.h>
#include <thrift/lib/cpp/test/loadgen/RNG.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/perf/cpp/ClientLoadConfig.h>

using namespace apache::thrift::async;

namespace apache {
namespace thrift {
namespace test {

const int kTimeout = 60000;

std::shared_ptr<ClientWorker2::Client> ClientWorker2::createConnection() {
  const std::shared_ptr<ClientLoadConfig>& config = getConfig();
  folly::AsyncSocket::UniquePtr socket;
  std::unique_ptr<RequestChannel, folly::DelayedDestruction::Destructor>
      channel;
  if (config->useSSL()) {
    std::shared_ptr<folly::SSLContext> context =
        std::make_shared<folly::SSLContext>();
    if (!config->trustedCAList().empty()) {
      context->loadTrustedCertificates(config->trustedCAList().c_str());
      context->setVerificationOption(
          folly::SSLContext::SSLVerifyPeerEnum::VERIFY);
    }

    if (!config->ciphers().empty()) {
      context->ciphers(config->ciphers());
    }

    if (!config->key().empty() && !config->cert().empty()) {
      context->loadCertificate(config->cert().c_str());
      context->loadPrivateKey(config->key().c_str());
    }

    socket = TAsyncSSLSocket::newSocket(context, ebm_.getEventBase());
    socket->connect(nullptr, *config->getAddress());
    // Loop once to connect
    ebm_.getEventBase()->loop();
  } else {
    socket = folly::AsyncSocket::newSocket(
        ebm_.getEventBase(), *config->getAddress());
  }
  HeaderClientChannel::Options options;
  if (!config->useHeaderProtocol()) {
    options.setClientType(THRIFT_FRAMED_DEPRECATED);
  }
  // Always use binary in loadtesting to get apples to apples comparison
  options.setProtocolId(apache::thrift::protocol::T_BINARY_PROTOCOL);
  auto headerChannel =
      HeaderClientChannel::newChannel(std::move(socket), std::move(options));
  if (config->zlib()) {
    apache::thrift::CompressionConfig compressionConfig;
    compressionConfig.codecConfig_ref().ensure().set_zlibConfig();
    headerChannel->setDesiredCompressionConfig(compressionConfig);
  }
  headerChannel->setTimeout(kTimeout);
  channel = std::move(headerChannel);

  return std::shared_ptr<ClientWorker2::Client>(
      new ClientWorker2::Client(std::move(channel)));
}

void ClientWorker2::performOperation(
    const std::shared_ptr<Client>& client, uint32_t opType) {
  switch (static_cast<ClientLoadConfig::OperationEnum>(opType)) {
    case ClientLoadConfig::OP_NOOP:
      return performNoop(client);
    case ClientLoadConfig::OP_ONEWAY_NOOP:
      return performOnewayNoop(client);
    case ClientLoadConfig::OP_ASYNC_NOOP:
      return performAsyncNoop(client);
    case ClientLoadConfig::OP_SLEEP:
      return performSleep(client);
    case ClientLoadConfig::OP_ONEWAY_SLEEP:
      return performOnewaySleep(client);
    case ClientLoadConfig::OP_BURN:
      return performBurn(client);
    case ClientLoadConfig::OP_ONEWAY_BURN:
      return performOnewayBurn(client);
    case ClientLoadConfig::OP_BAD_SLEEP:
      return performBadSleep(client);
    case ClientLoadConfig::OP_BAD_BURN:
      return performBadBurn(client);
    case ClientLoadConfig::OP_THROW_ERROR:
      return performThrowError(client);
    case ClientLoadConfig::OP_THROW_UNEXPECTED:
      return performThrowUnexpected(client);
    case ClientLoadConfig::OP_ONEWAY_THROW:
      return performOnewayThrow(client);
    case ClientLoadConfig::OP_SEND:
      return performSend(client);
    case ClientLoadConfig::OP_ONEWAY_SEND:
      return performOnewaySend(client);
    case ClientLoadConfig::OP_RECV:
      return performRecv(client);
    case ClientLoadConfig::OP_SENDRECV:
      return performSendrecv(client);
    case ClientLoadConfig::OP_ECHO:
      return performEcho(client);
    case ClientLoadConfig::OP_ADD:
      return performAdd(client);
    case ClientLoadConfig::OP_LARGE_CONTAINER:
      return performLargeContainer(client);
    case ClientLoadConfig::OP_ITER_ALL_FIELDS:
      return performIterAllFields(client);
    case ClientLoadConfig::NUM_OPS:
      // fall through
      break;
  }
  // no default case, so gcc will warn us if a new op is added
  // and this switch statement is not updated

  T_ERROR(
      "ClientWorker2::performOperation() got unknown operation %" PRIu32,
      opType);
  assert(false);
}

void ClientWorker2::performNoop(const std::shared_ptr<Client>& client) {
  client->sync_noop();
}

void ClientWorker2::performOnewayNoop(const std::shared_ptr<Client>& client) {
  client->sync_onewayNoop();
}

void ClientWorker2::performAsyncNoop(const std::shared_ptr<Client>& client) {
  client->sync_asyncNoop();
}

void ClientWorker2::performSleep(const std::shared_ptr<Client>& client) {
  client->sync_sleep(getConfig()->pickSleepUsec());
}

void ClientWorker2::performOnewaySleep(const std::shared_ptr<Client>& client) {
  client->sync_onewaySleep(getConfig()->pickSleepUsec());
}

void ClientWorker2::performBurn(const std::shared_ptr<Client>& client) {
  client->sync_burn(getConfig()->pickBurnUsec());
}

void ClientWorker2::performOnewayBurn(const std::shared_ptr<Client>& client) {
  client->sync_onewayBurn(getConfig()->pickBurnUsec());
}

void ClientWorker2::performBadSleep(const std::shared_ptr<Client>& client) {
  client->sync_badSleep(getConfig()->pickSleepUsec());
}

void ClientWorker2::performBadBurn(const std::shared_ptr<Client>& client) {
  client->sync_badBurn(getConfig()->pickBurnUsec());
}

void ClientWorker2::performThrowError(const std::shared_ptr<Client>& client) {
  uint32_t code = loadgen::RNG::getU32();
  try {
    client->sync_throwError(code);
    T_ERROR("throwError() didn't throw any exception");
  } catch (const LoadError& error) {
    DCHECK_EQ(*error.code_ref(), code);
  }
}

void ClientWorker2::performThrowUnexpected(
    const std::shared_ptr<Client>& client) {
  try {
    client->sync_throwUnexpected(loadgen::RNG::getU32());
    T_ERROR("throwUnexpected() didn't throw any exception");
  } catch (const TApplicationException&) {
    // expected; do nothing
  }
}

void ClientWorker2::performOnewayThrow(const std::shared_ptr<Client>& client) {
  client->sync_onewayThrow(loadgen::RNG::getU32());
}

void ClientWorker2::performSend(const std::shared_ptr<Client>& client) {
  std::string str(getConfig()->pickSendSize(), 'a');
  client->sync_send(str);
}

void ClientWorker2::performOnewaySend(const std::shared_ptr<Client>& client) {
  std::string str(getConfig()->pickSendSize(), 'a');
  client->sync_onewaySend(str);
}

void ClientWorker2::performRecv(const std::shared_ptr<Client>& client) {
  std::string str;
  client->sync_recv(str, getConfig()->pickRecvSize());
}

void ClientWorker2::performSendrecv(const std::shared_ptr<Client>& client) {
  std::string sendStr(getConfig()->pickSendSize(), 'a');
  std::string recvStr;
  client->sync_sendrecv(recvStr, sendStr, getConfig()->pickRecvSize());
}

void ClientWorker2::performEcho(const std::shared_ptr<Client>& client) {
  std::string sendStr(getConfig()->pickSendSize(), 'a');
  std::string recvStr;
  client->sync_echo(recvStr, sendStr);
}

void ClientWorker2::performAdd(const std::shared_ptr<Client>& client) {
  boost::uniform_int<int64_t> distribution;
  int64_t a = distribution(loadgen::RNG::getRNG());
  int64_t b = distribution(loadgen::RNG::getRNG());

  int64_t result = client->sync_add(a, b);

  if (result != a + b) {
    T_ERROR(
        "add(%" PRId64 ", %" PRId64 " gave wrong result %" PRId64
        "(expected %" PRId64 ")",
        a,
        b,
        result,
        a + b);
  }
}

void ClientWorker2::performLargeContainer(
    const std::shared_ptr<Client>& client) {
  std::vector<BigStruct> items;
  getConfig()->makeBigContainer<BigStruct>(items);
  client->sync_largeContainer(items);
}

void ClientWorker2::performIterAllFields(
    const std::shared_ptr<Client>& client) {
  std::vector<BigStruct> items;
  std::vector<BigStruct> out;
  getConfig()->makeBigContainer<BigStruct>(items);
  client->sync_iterAllFields(out, items);
  if (items != out) {
    T_ERROR("iterAllFields gave wrong result");
  }
}

} // namespace test
} // namespace thrift
} // namespace apache

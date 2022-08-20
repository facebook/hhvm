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

#include <thrift/perf/cpp/ClientLoadConfig.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <thrift/lib/cpp/test/loadgen/RNG.h>

DEFINE_string(server, "127.0.0.1", "Name/IP address of the server to test");
DEFINE_int32(port, 1234, "server port");
DEFINE_bool(framed, true, "use TFramedTransport");
DEFINE_bool(header, false, "use THeaderProtocol");
DEFINE_bool(async, false, "Use async client");
DEFINE_bool(ssl, false, "Use SSL");
DEFINE_bool(single_host, false, "Use Single Hosts option with Service Router");
DEFINE_string(key, "", "client SSL private key file");
DEFINE_string(cert, "", "client SSL certificate file");
DEFINE_string(trusted_ca_list, "", "file pointing to a trusted CA or list");
DEFINE_bool(use_tickets, true, "Use SSL session tickets on the client?");
DEFINE_bool(use_ssl_tfo, true, "Use TFO for SSL connections");
/*
 * Please refer to the online OpenSSL manual for the format of the string of
 * ciphers. The default cipher list is "ALL:!aNULL:!eNULL", which means all
 * ciphers except ones with no authentication (aNULL) or no encryption (eNULL).
 */
DEFINE_string(ciphers, "", "cipher suites supported by the client");

DEFINE_int32(num_threads, 5, "number of threads");
DEFINE_int64(qps, 0, "desired # of queries per second (0 for infinite)");
DEFINE_int32(
    ops_per_conn,
    1000,
    "number of operations to issue before opening a new connection");
DEFINE_int32(async_clients, 1, "number of simultaneous connections per thread");
DEFINE_int32(async_ops, 10, "number of oustanding async ops per connection");
DEFINE_bool(zlib, false, "use zlib compression");

// Relative weights for operation frequencies
DEFINE_int32(weight_noop, 0, "frequency weight for noop()");
DEFINE_int32(weight_oneway_noop, 0, "frequency weight for onewayNoop()");
DEFINE_int32(weight_async_noop, 0, "frequency weight for asyncNoop()");
DEFINE_int32(weight_sleep, 0, "frequency weight for sleep()");
DEFINE_int32(weight_oneway_sleep, 0, "frequency weight for onewaySleep()");
DEFINE_int32(weight_burn, 0, "frequency weight for burn()");
DEFINE_int32(weight_oneway_burn, 0, "frequency weight for onewayBurn()");
DEFINE_int32(weight_bad_sleep, 0, "frequency weight for badSleep()");
DEFINE_int32(weight_bad_burn, 0, "frequency weight for badBurn()");
DEFINE_int32(weight_throw_error, 0, "frequency weight for throwError()");
DEFINE_int32(
    weight_throw_unexpected, 0, "frequency weight for throwUnexpected()");
DEFINE_int32(weight_oneway_throw, 0, "frequency weight for onewayThrow()");
DEFINE_int32(weight_send, 0, "frequency weight for send()");
DEFINE_int32(weight_oneway_send, 0, "frequency weight for onewaySend()");
DEFINE_int32(weight_recv, 0, "frequency weight for recv()");
DEFINE_int32(weight_sendrecv, 0, "frequency weight for sendrecv()");
DEFINE_int32(weight_echo, 0, "frequency weight for echo()");
DEFINE_int32(weight_add, 0, "frequency weight for add()");
DEFINE_int32(
    weight_large_container, 0, "frequency weight for large_container()");
DEFINE_int32(
    weight_iter_all_fields, 0, "frequency weight for iter_all_fields()");

// Controls for how long sleep and burn operations should take
DEFINE_double(
    sleep_avg, 5000.0, "average # of microseconds for sleep operations");
DEFINE_double(
    sleep_sigma, -1.0, "log-normal sigma parameter for sleep duration");
DEFINE_double(
    burn_avg, 5000.0, "average # of microseconds for sleep operations");
DEFINE_double(
    burn_sigma, -1.0, "log-normal sigma parameter for sleep duration");
DEFINE_double(send_size_avg, 16384.0, "average # of bytes for send operations");
DEFINE_double(
    send_size_sigma, -1.0, "log-normal sigma parameter for send size");
DEFINE_double(
    recv_size_avg, 16384.0, "average # of bytes for receive operations");
DEFINE_double(
    recv_size_sigma, -1.0, "log-normal sigma parameter for receive size");
DEFINE_double(
    container_size_avg, 100.0, "average # of structs to put in a container");
DEFINE_double(
    container_size_sigma,
    -1.0,
    "log-normal sigma parameter for container size");
DEFINE_double(
    struct_field_size_avg, 64.0, "average length of field in BigStructs");
DEFINE_double(
    struct_field_size_sigma,
    -1.0,
    "log-normal sigma parameter for struct field size");

namespace apache {
namespace thrift {
namespace test {

ClientLoadConfig::ClientLoadConfig() : WeightedLoadConfig(NUM_OPS) {
  setOpInfo(OP_NOOP, "noop()", FLAGS_weight_noop);
  setOpInfo(OP_ONEWAY_NOOP, "onewayNoop()", FLAGS_weight_oneway_noop);
  setOpInfo(OP_ASYNC_NOOP, "asyncNoop()", FLAGS_weight_async_noop);
  setOpInfo(OP_SLEEP, "sleep()", FLAGS_weight_sleep);
  setOpInfo(OP_ONEWAY_SLEEP, "onewaySleep()", FLAGS_weight_oneway_sleep);
  setOpInfo(OP_BURN, "burn()", FLAGS_weight_burn);
  setOpInfo(OP_ONEWAY_BURN, "onewayBurn()", FLAGS_weight_oneway_burn);
  setOpInfo(OP_BAD_SLEEP, "badSleep()", FLAGS_weight_bad_sleep);
  setOpInfo(OP_BAD_BURN, "badBurn()", FLAGS_weight_bad_burn);
  setOpInfo(OP_THROW_ERROR, "throwError()", FLAGS_weight_throw_error);
  setOpInfo(
      OP_THROW_UNEXPECTED, "throwUnexpected()", FLAGS_weight_throw_unexpected);
  setOpInfo(OP_ONEWAY_THROW, "onewayThrow()", FLAGS_weight_oneway_throw);
  setOpInfo(OP_SEND, "send()", FLAGS_weight_send);
  setOpInfo(OP_ONEWAY_SEND, "onewaySleep()", FLAGS_weight_oneway_send);
  setOpInfo(OP_RECV, "recv()", FLAGS_weight_recv);
  setOpInfo(OP_SENDRECV, "sendrecv()", FLAGS_weight_sendrecv);
  setOpInfo(OP_ECHO, "echo()", FLAGS_weight_echo);
  setOpInfo(OP_ADD, "add()", FLAGS_weight_add);
  setOpInfo(
      OP_LARGE_CONTAINER, "large_container()", FLAGS_weight_large_container);
  setOpInfo(
      OP_ITER_ALL_FIELDS, "iter_all_fields()", FLAGS_weight_iter_all_fields);

  // Look up the hostname, and cache the result so we don't have to perform a
  // resolution for each connection attempt.
  address_.setFromHostPort(FLAGS_server.c_str(), FLAGS_port);
  if (FLAGS_server == "127.0.0.1") {
    char hostname[HOST_NAME_MAX + 1];
    PCHECK(!gethostname(hostname, sizeof(hostname)));
    addressHostname_ = hostname;
  } else {
    struct addrinfo hints, *res, *res0;
    char hostname[NI_MAXHOST] = {};

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_NUMERICHOST;
    PCHECK(
        !getaddrinfo(address_.getAddressStr().c_str(), nullptr, &hints, &res0));

    for (res = res0; res; res = res->ai_next) {
      if (0 ==
          getnameinfo(
              res->ai_addr,
              res->ai_addrlen,
              hostname,
              NI_MAXHOST,
              nullptr,
              0,
              NI_NAMEREQD)) {
        break;
      }
    }
    freeaddrinfo(res0);
    addressHostname_ = hostname;
  }

  // TODO: verify that at least one weight is non-zero
  // TODO: it would be nice to be able to load from a config file
}

uint32_t ClientLoadConfig::pickOpsPerConnection() {
  return FLAGS_ops_per_conn;
}

uint32_t ClientLoadConfig::getAsyncClients() const {
  return FLAGS_async_clients;
}

uint32_t ClientLoadConfig::getAsyncOpsPerClient() const {
  return FLAGS_async_ops;
}

uint32_t ClientLoadConfig::getNumWorkerThreads() const {
  return FLAGS_num_threads;
}

uint64_t ClientLoadConfig::getDesiredQPS() const {
  if (FLAGS_qps <= 0) {
    return 0;
  } else {
    return FLAGS_qps;
  }
}

uint32_t ClientLoadConfig::pickSleepUsec() {
  return pickLogNormal(FLAGS_sleep_avg, FLAGS_sleep_sigma);
}

uint32_t ClientLoadConfig::pickBurnUsec() {
  return pickLogNormal(FLAGS_burn_avg, FLAGS_burn_sigma);
}

uint32_t ClientLoadConfig::pickSendSize() {
  return pickLogNormal(FLAGS_send_size_avg, FLAGS_send_size_sigma);
}

uint32_t ClientLoadConfig::pickRecvSize() {
  return pickLogNormal(FLAGS_recv_size_avg, FLAGS_recv_size_sigma);
}

uint32_t ClientLoadConfig::pickContainerSize() {
  return pickLogNormal(FLAGS_container_size_avg, FLAGS_container_size_sigma);
}

uint32_t ClientLoadConfig::pickStructFieldSize() {
  return pickLogNormal(
      FLAGS_struct_field_size_avg, FLAGS_struct_field_size_sigma);
}

bool ClientLoadConfig::useFramedTransport() const {
  return FLAGS_framed;
}

bool ClientLoadConfig::useHeaderProtocol() const {
  return FLAGS_header;
}

bool ClientLoadConfig::useAsync() const {
  return FLAGS_async;
}

bool ClientLoadConfig::useSSL() const {
  return FLAGS_ssl;
}

bool ClientLoadConfig::useSSLTFO() const {
  return FLAGS_use_ssl_tfo;
}

bool ClientLoadConfig::useSingleHost() const {
  return FLAGS_single_host;
}

bool ClientLoadConfig::zlib() const {
  return FLAGS_zlib;
}

std::string ClientLoadConfig::server() const {
  return FLAGS_server;
}

uint32_t ClientLoadConfig::port() const {
  return FLAGS_port;
}

std::string ClientLoadConfig::key() const {
  return FLAGS_key;
}

std::string ClientLoadConfig::cert() const {
  return FLAGS_cert;
}

std::string ClientLoadConfig::trustedCAList() const {
  return FLAGS_trusted_ca_list;
}

std::string ClientLoadConfig::ciphers() const {
  return FLAGS_ciphers;
}

bool ClientLoadConfig::useTickets() const {
  return FLAGS_use_tickets;
}

uint32_t ClientLoadConfig::pickLogNormal(double mean, double sigma) {
  return static_cast<uint32_t>(loadgen::RNG::getLogNormal(mean, sigma));
}

} // namespace test
} // namespace thrift
} // namespace apache

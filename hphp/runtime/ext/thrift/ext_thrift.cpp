/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/ext/thrift/ext_thrift.h"

namespace HPHP { namespace thrift {

///////////////////////////////////////////////////////////////////////////////

Class* RpcOptions::c_RpcOptions = nullptr;

Object HHVM_METHOD(RpcOptions, setChunkBufferSize, int64_t chunk_buffer_size) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  if (UNLIKELY(chunk_buffer_size < 0)) {
    raise_warning("servicerouter: The chunk buffer size can't be negative (" +
      std::to_string(chunk_buffer_size) + "). The setting won't be changed.");
  } else if (UNLIKELY(chunk_buffer_size > std::numeric_limits<int32_t>::max())) {
    raise_warning("servicerouter: The chunk buffer size " +
      std::to_string(chunk_buffer_size) + " shouldn't exceed " +
      std::to_string(std::numeric_limits<int32_t>::max()) +
      ". The setting won't be changed.");
  } else {
    data->rpcOptions.setChunkBufferSize(chunk_buffer_size);
  }
  return Object(this_);
}

Object HHVM_METHOD(RpcOptions, setRoutingKey, const String& routing_key) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  data->rpcOptions.setRoutingKey(
    std::string(routing_key.c_str(), routing_key.size()));
  return Object(this_);
}

Object HHVM_METHOD(RpcOptions, setShardId, const String& shard_id) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  data->rpcOptions.setShardId(std::string(shard_id.c_str(), shard_id.size()));
  return Object(this_);
}

// TODO (partisan): Deprecate in favor of more clear for extrnal users setHeader
Object HHVM_METHOD(RpcOptions, setWriteHeader, const String& key, const String& value) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  data->rpcOptions.setWriteHeader(std::string(key.c_str(), key.size()),
    std::string(value.c_str(), value.size()));
  return Object(this_);
}

Object HHVM_METHOD(RpcOptions, setHeader, const String& key, const String& value) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  data->rpcOptions.setWriteHeader(std::string(key.c_str(), key.size()),
    std::string(value.c_str(), value.size()));
  return Object(this_);
}

String HHVM_METHOD(RpcOptions, __toString) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  std::string result("RpcOptions(");
  result += "chunkBufferSize: " +
    std::to_string(data->rpcOptions.getChunkBufferSize()) + "; ";
  result += "routingKey: \"" + data->rpcOptions.getRoutingKey() + "\"; ";
  result += "shardId: \"" + data->rpcOptions.getShardId() + "\"; ";
  result += "headers: {";
  bool first = true;
  for (const auto& it : data->rpcOptions.getWriteHeaders()) {
    if (!first) {
      result += ", ";
    } else {
      first = false;
    }
    result += "\"" + it.first + "\": \"" + it.second + "\"";
  }
  result += "}";
  result += ")\n";
  return result;
}

///////////////////////////////////////////////////////////////////////////////

static struct ThriftExtension final : Extension {
  ThriftExtension() : Extension("thrift_protocol", NO_EXTENSION_VERSION_YET) {}
  void moduleInit() override {
    HHVM_FE(thrift_protocol_write_binary);
    HHVM_FE(thrift_protocol_read_binary);
    HHVM_FE(thrift_protocol_read_binary_struct);
    HHVM_FE(thrift_protocol_set_compact_version);
    HHVM_FE(thrift_protocol_write_compact);
    HHVM_FE(thrift_protocol_read_compact);
    HHVM_FE(thrift_protocol_read_compact_struct);

    Native::registerNativeDataInfo<RpcOptions>(s_RpcOptions.get());
    HHVM_ME(RpcOptions, setChunkBufferSize);
    HHVM_ME(RpcOptions, setRoutingKey);
    HHVM_ME(RpcOptions, setShardId);
    HHVM_ME(RpcOptions, setWriteHeader);
    HHVM_ME(RpcOptions, __toString);

    loadSystemlib("thrift");
  }
} s_thrift_extension;

}}

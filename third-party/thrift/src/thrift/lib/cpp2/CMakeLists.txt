# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Set the cpp2 directory
set(LIB_CPP2_HOME ${CMAKE_CURRENT_SOURCE_DIR})


#######
# CMAKE variables only have local/subdirectory scope
# So even though this is defined in ../thrift/CMakeLists.txt as a variable
# it's not accessible here and silently defaults to ""
#
# Using global properties gets around this; probably better to restructure
# the directories/files to avoid this type of hack
#
get_property(any_rep-cpp2-SOURCES GLOBAL PROPERTY any_rep-cpp2-SOURCES)
get_property(RpcMetadata-cpp2-SOURCES GLOBAL PROPERTY RpcMetadata-cpp2-SOURCES)
get_property(metadata-cpp2-SOURCES GLOBAL PROPERTY metadata-cpp2-SOURCES)
get_property(frozen-cpp2-SOURCES GLOBAL PROPERTY frozen-cpp2-SOURCES)
get_property(reflection-cpp2-SOURCES GLOBAL PROPERTY reflection-cpp2-SOURCES)
get_property(RocketUpgrade-cpp2-SOURCES GLOBAL PROPERTY
  RocketUpgrade-cpp2-SOURCES)
get_property(id-cpp2-SOURCES GLOBAL PROPERTY id-cpp2-SOURCES)
get_property(standard-cpp2-SOURCES GLOBAL PROPERTY standard-cpp2-SOURCES)
get_property(type_rep-cpp2-SOURCES GLOBAL PROPERTY type_rep-cpp2-SOURCES)
get_property(type-cpp2-SOURCES GLOBAL PROPERTY type-cpp2-SOURCES)

bypass_source_check("${metadata-cpp2-SOURCES}")
add_library(
  thriftmetadata

  ${metadata-cpp2-SOURCES}
  gen/module_types_cpp.cpp
  protocol/CompactProtocol.cpp # can't link against thriftprotocol, dep cycle
  ../cpp/protocol/TProtocolException.cpp
  ../cpp/util/VarintUtils.cpp
)
add_dependencies(thriftmetadata metadata-cpp2-target)
target_link_libraries(
  thriftmetadata
  PUBLIC
    Folly::folly
)

bypass_source_check("${frozen-cpp2-SOURCES}")
add_library(
  thriftfrozen2

  frozen/Frozen.cpp
  frozen/FrozenUtil.cpp
  frozen/schema/MemorySchema.cpp
  ${frozen-cpp2-SOURCES}
)
add_dependencies(thriftfrozen2 frozen-cpp2-target thriftmetadata)
target_link_libraries(
  thriftfrozen2
  PUBLIC
    Folly::folly
    thriftmetadata
    thriftprotocol
    ${GLOG_LIBRARIES}
    ${LIBGFLAGS_LIBRARY}
)

bypass_source_check("${id-cpp2-SOURCES}")
bypass_source_check("${standard-cpp2-SOURCES}")
bypass_source_check("${type_rep-cpp2-SOURCES}")
add_library(
  thrifttyperep

  ${id-cpp2-SOURCES}
  ${standard-cpp2-SOURCES}
  ${type_rep-cpp2-SOURCES}
)
add_dependencies(thrifttyperep id-cpp2-target standard-cpp2-target
  type_rep-cpp2-target thriftannotation)
target_link_libraries(
  thrifttyperep
  PUBLIC
    Folly::folly
    thriftannotation
)

bypass_source_check("${type-cpp2-SOURCES}")
add_library(
  thrifttype

  ${type-cpp2-SOURCES}

  op/Serializer.cpp
  type/Any.cpp
  type/BaseType.cpp
  type/Protocol.cpp
  type/Runtime.cpp
  type/Type.cpp
  type/TypeRegistry.cpp
  type/UniversalName.cpp
)
add_dependencies(thrifttype type-cpp2-target thriftannotation thrifttyperep
  thriftprotocol)
target_link_libraries(
  thrifttype
  PUBLIC
    Folly::folly
    thriftannotation
    thrifttyperep
    thriftprotocol
)

bypass_source_check("${any_rep-cpp2-SOURCES}")
add_library(
  thriftanyrep

  ${any_rep-cpp2-SOURCES}
)
add_dependencies(thriftanyrep any_rep-cpp2-target thriftannotation thrifttype
  thrifttyperep)
target_link_libraries(
  thriftanyrep
  PUBLIC
    Folly::folly
    thriftannotation
    thrifttype
    thrifttyperep
)

bypass_source_check("${RpcMetadata-cpp2-SOURCES}")
add_library(
  rpcmetadata

  ${RpcMetadata-cpp2-SOURCES}
  gen/module_types_cpp.cpp
  protocol/CompactProtocol.cpp # can't link against thriftprotocol, dep cycle
  protocol/JSONProtocolCommon.cpp
  ../cpp/protocol/TBase64Utils.cpp
  ../cpp/protocol/TJSONProtocol.cpp
  ../cpp/protocol/TProtocolException.cpp
  ../cpp/util/VarintUtils.cpp
)
add_dependencies(rpcmetadata RpcMetadata-cpp2-target)
target_link_libraries(
  rpcmetadata
  PUBLIC
    Folly::folly
)

bypass_source_check("${reflection-cpp2-SOURCES}")
add_library(
  thriftprotocol

  protocol/Protocol.cpp
  protocol/BinaryProtocol.cpp
  protocol/CompactProtocol.cpp
  protocol/CompactV1Protocol.cpp
  protocol/DebugProtocol.cpp
  protocol/JSONProtocol.cpp
  protocol/JSONProtocolCommon.cpp
  protocol/Serializer.cpp
  protocol/TableBasedSerializer.cpp
  protocol/TableBasedSerializerImpl.cpp
  protocol/VirtualProtocol.cpp

  ../cpp/protocol/TDebugProtocol.cpp
  ../cpp/protocol/TJSONProtocol.cpp
  ../cpp/protocol/TBase64Utils.cpp
  ../cpp/protocol/TProtocolException.cpp
  ../cpp/protocol/TSimpleJSONProtocol.cpp
  ../cpp/util/VarintUtils.cpp
  ${reflection-cpp2-SOURCES}
)
add_dependencies(thriftprotocol reflection-cpp2-target)
target_link_libraries(
  thriftprotocol
  PUBLIC
    fmt::fmt
    thrift
    Folly::folly
    wangle::wangle
    ${GLOG_LIBRARIES}
    thrift-core
)

bypass_source_check("${RocketUpgrade-cpp2-SOURCES}")
add_library(
  thriftcpp2

  FieldRef.cpp
  Flags.cpp
  GeneratedCodeHelper.cpp
  TrustedServerException.cpp
  async/AsyncClient.cpp
  async/AsyncProcessor.cpp
  async/AsyncProcessorHelper.cpp
  async/ClientChannel.cpp
  async/ClientSinkBridge.cpp
  async/ClientStreamBridge.cpp
  async/Cpp2Channel.cpp
  async/DuplexChannel.cpp
  async/FramingHandler.cpp
  async/HeaderChannel.cpp
  async/HeaderChannelTrait.cpp
  async/HeaderClientChannel.cpp
  async/HeaderServerChannel.cpp
  async/HibernatingRequestChannel.cpp
  async/Interaction.cpp
  async/MultiplexAsyncProcessor.cpp
  async/PooledRequestChannel.cpp
  async/ReconnectingRequestChannel.cpp
  async/RequestCallback.cpp
  async/RequestChannel.cpp
  async/ResponseChannel.cpp
  async/RetryingRequestChannel.cpp
  async/RocketClientChannel.cpp
  async/RpcOptions.cpp
  async/RpcTypes.cpp
  async/ServerGeneratorStream.cpp
  async/ServerRequestData.cpp
  async/ServerSinkBridge.cpp
  async/ThreadBoundAdaptorChannel.cpp
  gen/module_metadata_cpp.cpp
  gen/module_types_cpp.cpp
  security/AsyncStopTLS.cpp
  security/FizzPeeker.cpp
  security/SSLUtil.cpp
  security/extensions/ThriftParametersClientExtension.cpp
  security/extensions/ThriftParametersContext.cpp
  security/extensions/Types.cpp
  server/AdaptiveConcurrency.cpp
  server/BaseThriftServer.cpp
  server/ConcurrencyControllerInterface.cpp
  server/ConcurrencyControllerBase.cpp
  server/Cpp2ConnContext.cpp
  server/Cpp2Connection.cpp
  server/Cpp2Worker.cpp
  server/CPUConcurrencyController.cpp
  server/IOUringUtil.cpp
  server/InternalPriorityRequestPile.cpp
  server/LoggingEvent.cpp
  server/LoggingEventHelper.cpp
  server/MonitoringMethodNames.cpp
  server/ParallelConcurrencyController.cpp
  server/TokenBucketConcurrencyController.cpp
  server/StandardConcurrencyController.cpp
  server/ThreadManagerLoggingWrapper.cpp
  server/RequestDebugLog.cpp
  server/RequestPileBase.cpp
  server/RequestPileInterface.cpp
  server/ReactiveToggle.cpp
  server/RequestsRegistry.cpp
  server/ResourcePool.cpp
  server/ResourcePoolSet.cpp
  server/ResourcePoolHandle.cpp
  server/RoundRobinRequestPile.cpp
  server/WeightedRequestPileQueue.cpp
  server/ServerAttribute.cpp
  server/ServerFlags.cpp
  server/ServerInstrumentation.cpp
  server/ServiceHealthPoller.cpp
  server/ThriftProcessor.cpp
  server/ThriftServer.cpp
  server/ThriftServerConfig.cpp
  server/TMConcurrencyController.cpp
  server/peeking/TLSHelper.cpp
  transport/core/RequestStateMachine.cpp
  transport/core/RpcMetadataUtil.cpp
  transport/core/RpcMetadataPlugins.cpp
  transport/core/ThriftClient.cpp
  transport/core/ThriftClientCallback.cpp
  transport/core/ThriftRequest.cpp
  transport/rocket/FdSocket.cpp
  transport/rocket/PayloadUtils.cpp
  transport/rocket/Types.cpp
  transport/rocket/Compression.cpp
  transport/rocket/client/RequestContext.cpp
  transport/rocket/client/RequestContextQueue.cpp
  transport/rocket/client/RocketClient.cpp
  transport/rocket/client/RocketStreamServerCallback.cpp
  transport/rocket/flush/FlushManager.cpp
  transport/rocket/framing/parser/AllocatingParserStrategy.cpp
  transport/rocket/framing/ErrorCode.cpp
  transport/rocket/framing/Frames.cpp
  transport/rocket/framing/Parser.cpp
  transport/rocket/framing/Serializer.cpp
  transport/rocket/framing/Util.cpp
  transport/rocket/server/RocketRoutingHandler.cpp
  transport/rocket/server/RocketServerConnection.cpp
  transport/rocket/server/RocketServerConnectionPlugins.cpp
  transport/rocket/server/RocketServerFrameContext.cpp
  transport/rocket/server/RocketSinkClientCallback.cpp
  transport/rocket/server/RocketStreamClientCallback.cpp
  transport/rocket/server/RocketThriftRequests.cpp
  transport/rocket/server/ThriftRocketServerHandler.cpp
  util/Checksum.cpp
  util/DebugString.cpp
  util/EmptyAsyncProcessor.cpp
  util/ScopedServerInterfaceThread.cpp
  util/ScopedServerThread.cpp
  visitation/visit_by_thrift_field_metadata.cpp
  ${RocketUpgrade-cpp2-SOURCES}
)
add_dependencies(
  thriftcpp2
  rpcmetadata
  RocketUpgrade-cpp2-target
  thriftannotation
  thriftmetadata
  thrifttyperep
  thrifttype
  thriftanyrep
)
target_link_libraries(
  thriftcpp2
  PUBLIC
    thrift
    rpcmetadata
    thriftannotation
    thriftmetadata
    thriftfrozen2
    thriftprotocol
    thrifttyperep
    thrifttype
    thriftanyrep
)

add_library(
  quic_thriftcpp2

  server/ThriftQuicServer.cpp
)
add_dependencies(
  quic_thriftcpp2

  thriftcpp2
  mvfst::mvfst_server_async_tran
)
target_link_libraries(
  quic_thriftcpp2

  PUBLIC
    thriftcpp2
    mvfst::mvfst_server_async_tran
)

# Install includes
set(THRIFT2_HEADER_DIRS
  async
  frozen
  protocol
  server
  transport
  util
)
install(
  DIRECTORY ${THRIFT2_HEADER_DIRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/thrift/lib/cpp2
  FILES_MATCHING PATTERN "*.h" PATTERN "*.tcc"
)

if (BUILD_SHARED_LIBS)
  set_target_properties(rpcmetadata thriftannotation thriftmetadata
    thriftfrozen2 thriftprotocol thriftcpp2 thrifttyperep thrifttype
    PROPERTIES VERSION ${PACKAGE_VERSION}
    )
endif()

install(
  TARGETS
    rpcmetadata
    thriftmetadata
    thriftfrozen2
    thrifttyperep
    thrifttype
    thriftanyrep
    thriftprotocol
    thriftcpp2
  EXPORT fbthrift-exports
  DESTINATION ${LIB_INSTALL_DIR}
)

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

if (THRIFT_LIB_ONLY OR build_all)
  add_subdirectory(schema)
endif ()

if (enable_tests)
  add_subdirectory(protocol/test)
  add_subdirectory(test)
endif ()

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
get_property(serverdbginfo-cpp2-SOURCES GLOBAL PROPERTY
  serverdbginfo-cpp2-SOURCES)
get_property(protocol-cpp2-SOURCES GLOBAL PROPERTY protocol-cpp2-SOURCES)
get_property(protocol_detail-cpp2-SOURCES GLOBAL PROPERTY
  protocol_detail-cpp2-SOURCES)
get_property(record-cpp2-SOURCES GLOBAL PROPERTY record-cpp2-SOURCES)

get_property(schema-cpp2-SOURCES GLOBAL PROPERTY schema-cpp2-SOURCES)
get_property(type_id-cpp2-SOURCES GLOBAL PROPERTY type_id-cpp2-SOURCES)
get_property(type_system-cpp2-SOURCES GLOBAL PROPERTY type_system-cpp2-SOURCES)
get_property(syntax_graph-cpp2-SOURCES GLOBAL PROPERTY
  syntax_graph-cpp2-SOURCES)

bypass_source_check("${type_id-cpp2-SOURCES}")
bypass_source_check("${protocol-cpp2-SOURCES}")
bypass_source_check("${protocol_detail-cpp2-SOURCES}")
bypass_source_check("${type_system-cpp2-SOURCES}")
bypass_source_check("${schema-cpp2-SOURCES}")
bypass_source_check("${record-cpp2-SOURCES}")
bypass_source_check("${syntax_graph-cpp2-SOURCES}")
bypass_source_check("${metadata-cpp2-SOURCES}")
add_library(
  thriftmetadata

  ${metadata-cpp2-SOURCES}
  gen/module_types_cpp.cpp
  protocol/CompactProtocol.cpp # can't link against thriftprotocol, dep cycle
  ../cpp/protocol/TProtocolException.cpp
  ../cpp/util/VarintUtils.cpp

  dynamic/SerializableRecord.cpp
  dynamic/SerializableTypeSystemBuilder.cpp
  dynamic/TypeId.cpp
  dynamic/TypeSystem.cpp
  dynamic/TypeSystemBuilder.cpp
  dynamic/TypeSystemTraits.cpp
  runtime/BaseSchemaRegistry.cpp
  runtime/Init.cpp
  runtime/SchemaRegistry.cpp
  schema/detail/Merge.cpp
  schema/detail/SchemaBackedResolver.cpp
  schema/SyntaxGraph.cpp
  ../thrift/detail/protocol.cpp
  ../thrift/detail/TypeIdAdapter.cpp
  ../thrift/detail/TypeSystemAdapter.cpp

  ${protocol-cpp2-SOURCES}
  ${protocol_detail-cpp2-SOURCES}
  ${type_id-cpp2-SOURCES}
  ${type_system-cpp2-SOURCES}
  ${schema-cpp2-SOURCES}
  ${record-cpp2-SOURCES}
  ${syntax_graph-cpp2-SOURCES}
)
add_dependencies(thriftmetadata metadata-cpp2-target
  protocol-cpp2-target
  protocol_detail-cpp2-target
  type_id-cpp2-target
  type_system-cpp2-target
  schema-cpp2-target
  record-cpp2-target
  syntax_graph-cpp2-target)
target_link_libraries(
  thriftmetadata
  PUBLIC
    Folly::folly
)

add_library(
  thrift_dynamic_value

  dynamic/Any.cpp
  dynamic/Binary.cpp
  dynamic/DynamicValue.cpp
  dynamic/List.cpp
  dynamic/Map.cpp
  dynamic/Set.cpp
  dynamic/String.cpp
  dynamic/Struct.cpp
  dynamic/Union.cpp
  dynamic/detail/ConcreteMapAny.cpp
  dynamic/detail/ConcreteMapBinary.cpp
  dynamic/detail/ConcreteMapBool.cpp
  dynamic/detail/ConcreteMapDouble.cpp
  dynamic/detail/ConcreteMapFloat.cpp
  dynamic/detail/ConcreteMapI16.cpp
  dynamic/detail/ConcreteMapI32.cpp
  dynamic/detail/ConcreteMapI64.cpp
  dynamic/detail/ConcreteMapI8.cpp
  dynamic/detail/ConcreteMapList.cpp
  dynamic/detail/ConcreteMapMap.cpp
  dynamic/detail/ConcreteMapSet.cpp
  dynamic/detail/ConcreteMapString.cpp
  dynamic/detail/ConcreteMapStruct.cpp
  dynamic/detail/ConcreteMapUnion.cpp
  dynamic/detail/ConcreteSet.cpp
  dynamic/detail/Datum.cpp
  dynamic/detail/DatumHash.cpp
)
add_dependencies(thrift_dynamic_value thriftmetadata thriftprotocol)
target_link_libraries(
  thrift_dynamic_value
  PUBLIC
    Folly::folly
    thriftmetadata
    thriftprotocol
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
add_dependencies(thrifttype any_rep-cpp2-target type-cpp2-target
  thriftannotation thrifttyperep thriftprotocol)
target_link_libraries(
  thrifttype
  PUBLIC
    common
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

bypass_source_check("${serverdbginfo-cpp2-SOURCES}")
add_library(
  serverdbginfo

  ${serverdbginfo-cpp2-SOURCES}
)
add_dependencies(serverdbginfo serverdbginfo-cpp2-target)
target_link_libraries(
  serverdbginfo
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
  ../cpp/protocol/TType.cpp
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
  async/AsyncProcessorFactory.cpp
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
  async/InteractionOverloadPolicy.cpp
  async/InterceptorFlags.cpp
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
  async/ServerBiDiStreamFactory.cpp
  async/ServerGeneratorStreamBridge.cpp
  async/ServerRequestData.cpp
  async/ServerSinkBridge.cpp
  async/ServerSinkFactory.cpp
  async/StreamPayload.cpp
  async/processor/AsyncProcessor.cpp
  async/processor/AsyncProcessorFunc.cpp
  async/processor/EventTask.cpp
  async/processor/GeneratedAsyncProcessorBase.cpp
  async/processor/HandlerCallback.cpp
  async/processor/HandlerCallbackBase.cpp
  async/processor/HandlerCallbackOneWay.cpp
  async/processor/RequestParams.cpp
  async/processor/RequestTask.cpp
  async/processor/ServerInterface.cpp
  async/processor/ServerRequest.cpp
  async/processor/ServerRequestHelper.cpp
  async/processor/ServerRequestTask.cpp
  async/processor/ServiceHandlerBase.cpp
  gen/module_constants_cpp.cpp
  gen/module_types_cpp.cpp
  security/AsyncStopTLS.cpp
  security/FizzPeeker.cpp
  security/SSLUtil.cpp
  security/extensions/ThriftParametersClientExtension.cpp
  security/extensions/Types.cpp
  server/AdaptiveConcurrency.cpp
  server/ConcurrencyControllerBase.cpp
  server/Cpp2ConnContext.cpp
  server/Cpp2Connection.cpp
  server/Cpp2Worker.cpp
  server/CPUConcurrencyController.cpp
  server/DecoratorData.cpp
  server/DecoratorDataRuntime.cpp
  server/IOUringUtil.cpp
  server/LazyDynamicArguments.cpp
  server/LegacyHeaderRoutingHandler.cpp
  server/LoggingEvent.cpp
  server/LoggingEventHelper.cpp
  server/LoggingEventTransportMetadata.cpp
  server/MonitoringMethodNames.cpp
  server/ParallelConcurrencyController.cpp
  server/TokenBucketConcurrencyController.cpp
  server/ServiceInterceptorQualifiedName.cpp
  server/StandardConcurrencyController.cpp
  server/ThreadManagerLoggingWrapper.cpp
  server/RequestDebugLog.cpp
  server/RequestPileBase.cpp
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
  server/ServiceInterceptorBase.cpp
  server/ServiceInterceptorControl.cpp
  server/ServiceInterceptorQualifiedName.cpp
  server/ThriftProcessor.cpp
  server/ThriftServer.cpp
  server/ThriftServerConfig.cpp
  server/TMConcurrencyController.cpp
  server/peeking/TLSHelper.cpp
  server/metrics/PendingConnectionsMetrics.cpp
  transport/core/RequestStateMachine.cpp
  transport/core/RpcMetadataUtil.cpp
  transport/core/RpcMetadataPlugins.cpp
  transport/core/SendCallbacks.cpp
  transport/core/ThriftClient.cpp
  transport/core/ThriftClientCallback.cpp
  transport/core/ThriftRequest.cpp
  transport/rocket/FdSocket.cpp
  transport/rocket/ChecksumGenerator.cpp
  transport/rocket/Types.cpp
  transport/rocket/compression/CompressionManager.cpp
  transport/rocket/compression/CompressionAlgorithmSelector.cpp
  transport/rocket/compression/CustomCompressorRegistry.cpp
  transport/rocket/client/RequestContext.cpp
  transport/rocket/client/RequestContextQueue.cpp
  transport/rocket/client/RocketClient.cpp
  transport/rocket/client/KeepAliveWatcher.cpp
  transport/rocket/client/RocketStreamServerCallback.cpp
  transport/rocket/client/RocketStreamServerCallbackWithChunkTimeout.cpp
  transport/rocket/client/RocketSinkServerCallback.cpp
  transport/rocket/client/RocketBiDiServerCallback.cpp
  transport/rocket/flush/FlushManager.cpp
  transport/rocket/framing/parser/AllocatingParserStrategy.cpp
  transport/rocket/framing/ErrorCode.cpp
  transport/rocket/framing/FrameDataFirstFieldAligner.cpp
  transport/rocket/framing/Frames.cpp
  transport/rocket/framing/Parser.cpp
  transport/rocket/framing/Serializer.cpp
  transport/rocket/framing/Util.cpp
  transport/rocket/payload/DefaultPayloadSerializerStrategy.cpp
  transport/rocket/payload/PayloadSerializer.cpp
  transport/rocket/server/InteractionOverload.cpp
  transport/rocket/server/RocketRoutingHandler.cpp
  transport/rocket/server/RocketServerConnection.cpp
  transport/rocket/server/RefactoredRocketServerConnection.cpp
  transport/rocket/server/RocketServerConnectionPlugins.cpp
  transport/rocket/server/RocketServerFrameContext.cpp
  transport/rocket/server/RocketSinkClientCallback.cpp
  transport/rocket/server/RocketStreamClientCallback.cpp
  transport/rocket/server/RocketBiDiClientCallback.cpp
  transport/rocket/server/RocketThriftRequests.cpp
  transport/rocket/server/RocketServerConnectionFactory.cpp
  transport/rocket/server/ThriftRocketServerHandler.cpp
  transport/rocket/server/detail/WriteBatcher.cpp
  util/Checksum.cpp
  util/DebugString.cpp
  util/EmptyAsyncProcessor.cpp
  util/SchemaToMetadata.cpp
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
  thrift_dynamic_value
  thrifttyperep
  thrifttype
  thriftanyrep
  serverdbginfo
)

target_link_libraries(
  thriftcpp2
  PUBLIC
    fmt::fmt
    thrift
    rpcmetadata
    thriftannotation
    thriftmetadata
    thrift_dynamic_value
    thriftfrozen2
    thriftprotocol
    thrifttyperep
    thrifttype
    thriftanyrep
    serverdbginfo
    ${Xxhash_LIBRARY}
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
    thrift_dynamic_value thriftfrozen2 thriftprotocol thriftcpp2 thrifttyperep thrifttype
    serverdbginfo
    PROPERTIES VERSION ${PACKAGE_VERSION}
    )
endif()

install(
  TARGETS
    rpcmetadata
    thriftmetadata
    thrift_dynamic_value
    thriftfrozen2
    thrifttyperep
    thrifttype
    thriftanyrep
    thriftprotocol
    thriftcpp2
    serverdbginfo
  EXPORT fbthrift-exports
  DESTINATION ${LIB_INSTALL_DIR}
)

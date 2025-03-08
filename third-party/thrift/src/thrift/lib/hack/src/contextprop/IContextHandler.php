<?hh
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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
 *
 */

interface IContextHandler {
  /**
   * WWW is the client and is receiving a response from an external thrift service.
   * Use this hook to update ThriftContextPropState based on the incoming
   * ThriftFrameworkMetadata header.
   */
  public function onIncomingDownstream(
    ThriftContextPropState $mutable_ctx,
    ClientInstrumentationParams $params,
    ImmutableThriftFrameworkMetadataOnResponse $immutable_tfmr,
  ): void;

  /**
   * WWW is the client and is sending a request to an external thrift service.
   * Use this hook to update outgoing ThriftFrameworkMetadata header based on
   * the current ThriftContextPropState.
   */
  public function onOutgoingDownstream(
    ClientInstrumentationParams $params,
    ThriftFrameworkMetadata $mutable_tfm,
    ImmutableThriftContextPropState $immutable_ctx,
  ): void;

  /**
   * WWW is the server and is receiving a request from an external thrift
   * service. Use this hook to update ThriftContextPropState based on the
   * incoming ThriftFrameworkMetadata header.
   */
  public function onIncomingUpstream(
    ThriftContextPropState $mutable_ctx,
    ServerInstrumentationParams $params,
    ImmutableThriftFrameworkMetadata $immutable_tfm,
  ): void;

  /**
   * WWW is the server and is sending a response to an external thrift service.
   * Use this hook to update outgoing ThriftFrameworkMetadata header based on
   * the current ThriftContextPropState.
   */
  public function onOutgoingUpstream(
    ServerInstrumentationParams $params,
    ThriftFrameworkMetadataOnResponse $mutable_tfmr,
    ImmutableThriftContextPropState $immutable_ctx,
  ): void;

}

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

<<Oncalls('xdc_artillery')>>
final class ProductIdContextHandlerTest extends WWWTest {
  use MethodLevelTest;

  private static function setMCPOriginId(MCPOriginIDs $origin_id): void {
    MCPContext::setGlobal__UNSAFE(
      $origin_id,
      WwwHackTestAssetXID::get(nameof static),
    );
  }

  public async function testProductIdDifferentFromTFM(): Awaitable<void> {
    $params = shape();
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();
    $mutable_tfm->origin_id = MCPOriginIDs::XI_INTEL;

    // TagManager::getLatestOriginID() returns a different value than what is in TFM already
    $mock =
      self::mockClassStaticMethodUNSAFE(TagManager::class, 'getLatestOriginID')
        ->mockReturn(MCPOriginIDs::CROSSMETA_INTEGRITY_XI);

    // call handler
    $handler = new ProductIdContextHandler();
    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );

    // verify that TagManager::getLatestOriginID() was called

    expect($mock)->wasCalledOnce();

    // verify that TFM was updated
    expect($mutable_tfm->origin_id)->toEqual(
      MCPOriginIDs::CROSSMETA_INTEGRITY_XI,
    );
  }

  public async function testProductIdSameAsTFM(): Awaitable<void> {
    $params = shape();
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();
    $mutable_tfm->origin_id = MCPOriginIDs::XI_INTEL;

    // TagManager::getLatestOriginID() returns a different value than what is in TFM already
    $mock =
      self::mockClassStaticMethodUNSAFE(TagManager::class, 'getLatestOriginID')
        ->mockReturn(MCPOriginIDs::XI_INTEL);

    // call handler
    $handler = new ProductIdContextHandler();
    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );

    // verify that TagManager::getLatestOriginID() was called

    expect($mock)->wasCalledOnce();

    // verify that TFM was NOT updated
    expect($mutable_tfm->origin_id)->toEqual(MCPOriginIDs::XI_INTEL);
  }

  public async function testRegisteredHandlerOverridesProductIdWhenProductIdChanges(
  ): Awaitable<void> {
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    // product id in ThriftContextPropState is set
    self::setMCPOriginId(MCPOriginIDs::XI_INTEL);

    $transport =
      TServiceRouterTransport::create('sample_service_name', dict[], dict[]);
    $client_handler = new TContextPropV2ClientHandler($transport, $params);

    // TagManager::getLatestOriginID() returns a different value than what is in ThriftContextPropState already
    $mock =
      self::mockClassStaticMethodUNSAFE(TagManager::class, 'getLatestOriginID')
        ->mockReturn(MCPOriginIDs::CROSSMETA_INTEGRITY_XI);

    $handler = new ProductIdContextHandler();
    $client_handler->addHandler($handler);
    $client_handler->preSend(
      'sample_method_name',
      null,
      0,
      'sample_service_interface',
    );

    // verify that TagManager::getLatestOriginID() was called
    expect($mock)->wasCalledOnce();

    // verify that product ID in TFM was updated
    $write_headers = $transport->getWriteHeaders();
    expect($write_headers)->toContainKey(
      ThriftFrameworkMetadata_CONSTANTS::ThriftFrameworkMetadataHeaderKey,
    );
    $encoded_request_tfm = $write_headers[
      ThriftFrameworkMetadata_CONSTANTS::ThriftFrameworkMetadataHeaderKey
    ];

    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->read(
      new TCompactProtocolAccelerated(
        new TMemoryBuffer(Base64::decode($encoded_request_tfm)),
      ),
    );
    expect($tfm->origin_id)->toEqual(MCPOriginIDs::CROSSMETA_INTEGRITY_XI);
  }

  public async function testRegisteredHandlerDoesNotOverrideProductIdWhenProductIdDoesNotChange(
  ): Awaitable<void> {
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    // product id in ThriftContextPropState is set
    self::setMCPOriginId(MCPOriginIDs::XI_INTEL);

    $transport =
      TServiceRouterTransport::create('sample_service_name', dict[], dict[]);
    $client_handler = new TContextPropV2ClientHandler($transport, $params);

    // TagManager::getLatestOriginID() returns the same value as what is in ThriftContextPropState already
    $mock =
      self::mockClassStaticMethodUNSAFE(TagManager::class, 'getLatestOriginID')
        ->mockReturn(MCPOriginIDs::XI_INTEL);

    $handler = new ProductIdContextHandler();
    $client_handler->addHandler($handler);
    $client_handler->preSend(
      'sample_method_name',
      null,
      0,
      'sample_service_interface',
    );

    // verify that TagManager::getLatestOriginID() was called
    expect($mock)->wasCalledOnce();

    // verify that product ID in TFM was updated
    $write_headers = $transport->getWriteHeaders();
    expect($write_headers)->toContainKey(
      ThriftFrameworkMetadata_CONSTANTS::ThriftFrameworkMetadataHeaderKey,
    );
    $encoded_request_tfm = $write_headers[
      ThriftFrameworkMetadata_CONSTANTS::ThriftFrameworkMetadataHeaderKey
    ];

    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->read(
      new TCompactProtocolAccelerated(
        new TMemoryBuffer(Base64::decode($encoded_request_tfm)),
      ),
    );
    expect($tfm->origin_id)->toEqual(MCPOriginIDs::XI_INTEL);
  }
}

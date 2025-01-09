<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('xdc_artillery')>>
final class ProductIdContextHandlerTest extends WWWTest {
  use MethodLevelTest;

  public async function testProductIdDifferentFromTFM(): Awaitable<void> {
    $params = shape();
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();
    $mutable_tfm->origin_id = 123;

    // TagManager::getLatestOriginID() returns a different value than what is in TFM already
    $mock =
      self::mockClassStaticMethodUNSAFE(TagManager::class, 'getLatestOriginID')
        ->mockReturn(456);

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
    expect($mutable_tfm->origin_id)->toEqual(456);
  }

  public async function testProductIdSameAsTFM(): Awaitable<void> {
    $params = shape();
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();
    $mutable_tfm->origin_id = 123;

    // TagManager::getLatestOriginID() returns a different value than what is in TFM already
    $mock =
      self::mockClassStaticMethodUNSAFE(TagManager::class, 'getLatestOriginID')
        ->mockReturn(123);

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
    expect($mutable_tfm->origin_id)->toEqual(123);
  }

  public async function testRegisteredHandlerOverridesProductIdWhenProductIdChanges(
  ): Awaitable<void> {
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    // product id in ThriftContextPropState is 123
    ThriftContextPropState::get()->setOriginId(123);

    $transport =
      TServiceRouterTransport::create('sample_service_name', dict[], dict[]);
    $client_handler = new TContextPropV2ClientHandler($transport, $params);

    // TagManager::getLatestOriginID() returns a different value than what is in ThriftContextPropState already
    $mock =
      self::mockClassStaticMethodUNSAFE(TagManager::class, 'getLatestOriginID')
        ->mockReturn(456);

    $handler = new ProductIdContextHandler();
    $client_handler->addHandler($handler);
    $client_handler->preSend('sample_method_name', null, 0);

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
    expect($tfm->origin_id)->toEqual(456);
  }

  public async function testRegisteredHandlerDoesNotOverrideProductIdWhenProductIdDoesNotChange(
  ): Awaitable<void> {
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    // product id in ThriftContextPropState is 123
    ThriftContextPropState::get()->setOriginId(123);

    $transport =
      TServiceRouterTransport::create('sample_service_name', dict[], dict[]);
    $client_handler = new TContextPropV2ClientHandler($transport, $params);

    // TagManager::getLatestOriginID() returns the same value as what is in ThriftContextPropState already
    $mock =
      self::mockClassStaticMethodUNSAFE(TagManager::class, 'getLatestOriginID')
        ->mockReturn(123);

    $handler = new ProductIdContextHandler();
    $client_handler->addHandler($handler);
    $client_handler->preSend('sample_method_name', null, 0);

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
    expect($tfm->origin_id)->toEqual(123);
  }

  public async function testRegisteredHandlerDoesOverrideProductIDWhenProductIdIsNull(
  ): Awaitable<void> {
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    // product id in ThriftContextPropState is null
    ThriftContextPropState::get()->setOriginId(null);

    $transport =
      TServiceRouterTransport::create('sample_service_name', dict[], dict[]);
    $client_handler = new TContextPropV2ClientHandler($transport, $params);

    // TagManager::getLatestOriginID() returns the same value as what is in ThriftContextPropState already
    $mock =
      self::mockClassStaticMethodUNSAFE(TagManager::class, 'getLatestOriginID')
        ->mockReturn(123);

    $handler = new ProductIdContextHandler();
    $client_handler->addHandler($handler);
    $client_handler->preSend('sample_method_name', null, 0);

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
    expect($tfm->origin_id)->toEqual(123);
  }
}

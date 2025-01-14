<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('xdc_artillery')>>
final class UpdateUniverseContextHandlerTest extends WWWTest {

  use ClassLevelTest;

  <<__Override>>
  public async function beforeEach(): Awaitable<void> {
    clear_class_memoization(PrivacyLibKS::class, 'isKilled');
    MockJustKnobs::overrideKnob<bool>(
      'www/privacylib:killswitch',
      false,
      null,
      dict[PLKS::XSU_UNIVERSE_CONTEXT_PROP => false],
    );
    PrivacyLibTestsUtils::clearArtifactLoaderMemoization();

    ThriftContextPropState::get()->clear();
    PZ2RequestLevelState::clear_FOR_TESTING_ONLY();
    MockPrivacyLibKS::restore();
  }

  public static function dataProvider(): dict<string, shape(
    'thrift_service_name' => string,
    'thrift_method_name' => string,
    'expected_universe_designator' => UniverseDesignator,
  )> {
    return dict[
      'test privacy lib artifact not found / null universe' => shape(
        'thrift_service_name' => 'thrift_name_INVALID',
        'thrift_method_name' => 'function_name_INVALID',
        'expected_universe_designator' =>
          UniverseDesignator::withXsu(Universe\XSUIdentifier::FACEBOOK),
      ),
      // METAEntMultiverse is multi-verse which result in dynamic propagation
      'test based on asset multiverse, dynamic propagation' => shape(
        'thrift_service_name' => 'MapleService',
        'thrift_method_name' => 'getSerializedObjects',
        'expected_universe_designator' =>
          UniverseDesignator::withXsu(Universe\XSUIdentifier::FACEBOOK),
      ),
      // InstagramTestUsersService is IGEntUniverse, static propagation
      'test based on asset universe, static propagation' => shape(
        'thrift_service_name' => 'InstagramTestUsersService',
        'thrift_method_name' => 'configureTestUser',
        'expected_universe_designator' =>
          UniverseDesignator::withXsu(Universe\XSUIdentifier::INSTAGRAM),
      ),
    ];
  }

  public async function testUpdatedToRequestSinceAssetInvalid(
  ): Awaitable<void> {
    await $this->genInitPZ2();
    $this->assertNullUniverse();

    $handler = new UpdateUniverseContextHandler();

    $params = shape(
      'service_name' =>
        "service_name_holder_to_be_updated_for_universe_propagation",
      'fn_name' =>
        "function_name_holder_to_be_updated_for_universe_propagation",
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    $immutable_ctx =
      new ImmutableThriftContextPropState(ThriftContextPropState::get());
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();

    $handler->onOutgoingDownstream($params, $mutable_tfm, $immutable_ctx);

    $this->assertUniverse(
      UniverseDesignator::withXsu(Universe\XSUIdentifier::FACEBOOK),
      $mutable_tfm,
    );
  }

  <<DataProvider('dataProvider')>>
  public async function testRegisterHandler(
    classname<IThriftClient> $thrift_service_name,
    string $thrift_method_name,
    UniverseDesignator $expected_universe_designator,
  ): Awaitable<void> {
    await $this->genInitPZ2();

    $this->assertNullUniverse();
    $handler = new UpdateUniverseContextHandler();

    $params = shape(
      'thrift_class' => $thrift_service_name,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );
    $transport =
      TServiceRouterTransport::create($thrift_service_name, dict[], dict[]);
    $client_handler = new TContextPropV2ClientHandler($transport, $params);

    $client_handler->addHandler($handler);
    $client_handler->preSend($thrift_method_name, null, 0);

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

    $this->assertUniverse($expected_universe_designator, $tfm);
  }

  public async function testUniverseContextPropKillswitch(): Awaitable<void> {
    $mock = self::mockClassStaticMethodUNSAFE(
      UpdateUniverseContextHandler::class,
      'getUniverseForPropagation',
    );

    $handler = new UpdateUniverseContextHandler();
    $params = shape(
      'service_name' =>
        "service_name_holder_to_be_updated_for_universe_propagation",
      'fn_name' =>
        "function_name_holder_to_be_updated_for_universe_propagation",
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );
    $immutable_ctx =
      new ImmutableThriftContextPropState(ThriftContextPropState::get());
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();

    $handler->onOutgoingDownstream($params, $mutable_tfm, $immutable_ctx);
    expect($mock)->wasCalled();

    $mock->clearCalls();
    clear_class_memoization(PrivacyLibKS::class, 'isKilled');
    MockJustKnobs::overrideKnob<bool>(
      'www/privacylib:killswitch',
      false,
      null,
      dict['xsu_universe_context_prop' => true],
    );

    $handler = new UpdateUniverseContextHandler();
    $params = shape(
      'service_name' =>
        "service_name_holder_to_be_updated_for_universe_propagation",
      'fn_name' =>
        "function_name_holder_to_be_updated_for_universe_propagation",
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );
    $immutable_ctx =
      new ImmutableThriftContextPropState(ThriftContextPropState::get());
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();

    $handler->onOutgoingDownstream($params, $mutable_tfm, $immutable_ctx);

    expect($mock)->wasNotCalled();

  }

  private async function genInitPZ2(): Awaitable<void> {
    await MockPZ2::genInitRequest(
      PZ2CodeAnnotationStatic::get()->withPolicy(
        PZ2XSUPolicy::create(PZ2XSUStates::FACEBOOK),
      ),
    );
  }

  private function assertNullUniverse(): void {
    $context_prop_state = ThriftContextPropState::get();
    expect($context_prop_state)->toNotBeNull();
    expect($context_prop_state->getPrivacyUniverseDesignator())->toBeNull();
  }

  private function assertUniverse(
    UniverseDesignator $universe,
    ThriftFrameworkMetadata $mutable_tfm,
  ): void {
    $universe_from_tfm = $mutable_tfm->privacyUniverse;
    expect($universe_from_tfm)->toNotBeNull();
    expect(UniverseDesignator::fromInt($universe_from_tfm as nonnull)->getXSU())
      ->toEqual($universe->getXSU());
  }
}

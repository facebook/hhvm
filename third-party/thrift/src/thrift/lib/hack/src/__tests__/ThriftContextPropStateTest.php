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

<<Oncalls('thrift')>>
final class ThriftContextPropStateTest extends WWWTest {
  use ClassLevelTest;

  <<__Override>>
  public static function configPrivacy(
    this::TIAssetConfig $config,
  )[write_props]: void {
    $config->zoneConfig()->policies(
      PLGDAConfig::schema()->labels(
        GovernableAnnotation::fromEnum(
          GovernableAnnotation\GovernableDataAnnotation::FB_DELEGATE_PAGE_RESTRICTED_INPUT_CLOSURE,
        ),
        GovernableAnnotation::fromEnum(
          GovernableAnnotation\GovernableDataAnnotation::FB_DELEGATE_PAGE_RESTRICTED_OUTPUT_CLOSURE,
        ),
      ),
    );

  }

  public static function dataProviderForInvalidUserIds(): dict<string, (int)> {
    return dict[
      'zero user id' => tuple(0),
      'negative user id' => tuple(-123),
    ];
  }
  <<DataProvider('dataProviderForInvalidUserIds')>>
  public function testUpdateIGUserIdWithInvalidId___DPRS_ACH_TEST(
    int $invalid_id,
  ): void {
    // Arrange
    $tcps = ThriftContextPropState::get();
    $tcps->clear();
    // Mock to ensure any positive ID is considered valid for the initial setup
    self::mockFunction(IgidUtils::isUserFbid<>)->mockReturn(true);
    $initial_valid_id = 54321;
    ThriftContextPropState::updateIGUserId(
      $initial_valid_id,
      'test_setup',
      UserIdSource::VIEWER_CONTEXT,
    );
    expect($tcps->getIGUserId())->toEqual($initial_valid_id);

    // Act
    $result = ThriftContextPropState::updateIGUserId(
      $invalid_id,
      'test_invalid',
      UserIdSource::VIEWER_CONTEXT,
    );

    // Assert
    // The original code should reject the invalid (zero or negative) ID and not change the state.
    // The mutant version accepts any non-null ID, so it will update the state and this test will fail.
    expect($result)->toBeFalse();
    expect($tcps->getIGUserId())->toEqual($initial_valid_id);
  }

  public function testClearDoesNotClearSerializedCache___DPRS_ACH_TEST(): void {
    $tcps = ThriftContextPropState::get();
    $tcps->clear();

    // Set some state and serialize it to populate the cache
    $tcps->setRequestId('test_request_id');
    $serialized_before_clear = $tcps->getSerialized();

    // Clear the state
    $tcps->clear();

    // Get the serialized state again
    $serialized_after_clear = $tcps->getSerialized();

    // In the correct implementation, clear() also clears the serialized cache,
    // so the new serialized string would represent an empty state and be
    // different from the one before clearing.
    // In the mutated version, the cache is not cleared, so getSerialized()
    // returns the stale data from before the clear() call. This assertion
    // will fail for the mutant.
    expect($serialized_before_clear)->toNotEqual($serialized_after_clear);

    // Additionally, the serialized state after clearing should be equal to
    // the serialized state of a newly cleared instance.
    $fresh_tcps = ThriftContextPropState::get();
    $fresh_tcps->clear();
    $serialized_empty = $fresh_tcps->getSerialized();
    expect($serialized_after_clear)->toEqual($serialized_empty);
  }

  public function testAddExperimentIdDirtiesCache___DPRS_ACH_TEST(): void {
    $tcps = ThriftContextPropState::get();
    $tcps->clear();

    // Serialize the initial state to populate the cache.
    $serialized_before = $tcps->getSerialized();

    // Add an experiment ID, which should modify the state.
    $tcps->addExperimentId(123);

    // Serialize the state again.
    $serialized_after = $tcps->getSerialized();

    // In the correct implementation, addExperimentId calls dirty(),
    // which clears the cache. The new serialization should reflect the
    // added experiment ID and be different from the original.
    // In the mutant version, dirty() is not called, so the cached
    // (stale) value is returned, and this assertion will fail.
    expect($serialized_before)->toNotEqual($serialized_after);
  }

  public function testAccess(): void {
    $tcps = ThriftContextPropState::get();
    expect($tcps->getRequestId())->toEqual("");
    $tcps->setRequestId("12345");
    expect(ThriftContextPropState::get()->getRequestId())->toEqual("12345");
  }

  public function testRegionalizationEntityNullable(): void {
    $tcps = ThriftContextPropState::get();
    expect($tcps->getRegionalizationEntity())->toBeNull();

    // 0 is different from null
    $tcps->setRegionalizationEntity(0);
    expect($tcps->getRegionalizationEntity())->toEqual(0);

    // override existing value
    $tcps->setRegionalizationEntity(1);
    expect($tcps->getRegionalizationEntity())->toEqual(1);

    // back to null
    $tcps->setRegionalizationEntity(null);
    expect($tcps->getRegionalizationEntity())->toBeNull();
  }

  public function testUserIdsNullable(): void {
    self::mockFunction(IgidUtils::isUserFbid<>)->mockReturn(true);
    $tcps = ThriftContextPropState::get();

    // override existing value
    expect(
      ThriftContextPropState::updateFBUserId(
        1,
        "test",
        UserIdSource::VIEWER_CONTEXT,
      ),
    )->toBeTrue();
    expect(
      ThriftContextPropState::updateIGUserId(
        2,
        "test",
        UserIdSource::VIEWER_CONTEXT,
      ),
    )->toBeTrue();
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(1);
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(2);

    // do not allow null to override existing value
    expect(ThriftContextPropState::updateFBUserId(
      null,
      "test",
      UserIdSource::VIEWER_CONTEXT,
    ))->toBeFalse();
    expect(ThriftContextPropState::updateIGUserId(
      null,
      "test",
      UserIdSource::VIEWER_CONTEXT,
    ))->toBeFalse();
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(1);
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(2);

    // set FB Id only
    expect(
      ThriftContextPropState::updateFBUserId(
        3,
        "test",
        UserIdSource::VIEWER_CONTEXT,
      ),
    )->toBeTrue();
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(3);
    expect($tcps->getFBUserId())->toEqual(3);
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(2);

    // set IG Id only
    expect(
      ThriftContextPropState::updateIGUserId(
        4,
        "test",
        UserIdSource::VIEWER_CONTEXT,
      ),
    )->toBeTrue();
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(3);
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(4);
    expect($tcps->getIGUserId())->toEqual(4);

  }

  public function testOfflineJobLocalContext(): void {
    $tcps = ThriftContextPropState::get();

    $tcps->setOfflineJobLocalContext(
      Gojira_OfflineJobLocalContext::withDefaultValues(),
    );
    expect($tcps->getOfflineJobLocalContext())->toNotBeNull();
    expect($tcps->getOfflineJobLocalContext()?->platformRunId)->toBeNull();
    expect($tcps->getOfflineJobLocalContext()?->anchorInstanceId)->toBeNull();
    expect($tcps->getOfflineJobLocalContext()?->leafRunId)->toBeNull();

    // override existing value
    $offline_job_local_context = new Gojira_OfflineJobLocalContext(
      Gojira_PlatformRunId::withDefaultValues(),
      Gojira_AnchorInstanceId::withDefaultValues(),
      Gojira_LeafRunId::withDefaultValues(),
    );
    $tcps->setOfflineJobLocalContext($offline_job_local_context);
    expect($tcps->getOfflineJobLocalContext()?->platformRunId)->toNotBeNull();
    expect($tcps->getOfflineJobLocalContext()?->anchorInstanceId)
      ->toNotBeNull();
    expect($tcps->getOfflineJobLocalContext()?->leafRunId)->toNotBeNull();
  }

  public function testOfflineJobCompactLocalContext(): void {
    $tcps = ThriftContextPropState::get();

    $tcps->setOfflineJobCompactLocalContext(
      Gojira_OfflineJobCompactLocalContext::withDefaultValues(),
    );
    expect($tcps->getOfflineJobCompactLocalContext())->toNotBeNull();
    expect($tcps->getOfflineJobCompactLocalContext()?->Ctx)->toEqual('');

    // override existing value
    $offline_job_compact_local_context =
      new Gojira_OfflineJobCompactLocalContext('test_binary');
    $tcps->setOfflineJobCompactLocalContext($offline_job_compact_local_context);

    expect($tcps->getOfflineJobCompactLocalContext()?->Ctx)->toEqual(
      'test_binary',
    );
  }

  public function testBaggage(): void {
    $tcps = ThriftContextPropState::get();
    $tcps->clear();
    expect($tcps->getBaggage())->toBeNull();
    $baggage = ContextProp\Baggage::withDefaultValues();
    $tcps->setBaggage($baggage);
    expect($tcps->getBaggage()?->regionalization_entity)->toBeNull();

    // Baggage with RE=32
    $baggage = ContextProp\Baggage::fromShape(shape(
      'regionalization_entity' => 32,
    ));
    $tcps->setBaggage($baggage);
    expect($tcps->getBaggage()?->regionalization_entity)->toEqual(32);
  }

  public function testTraceContext(): void {
    $tcps = ThriftContextPropState::get();
    $tcps->clear();
    expect($tcps->getBaggage())->toBeNull();
    $baggage = ContextProp\Baggage::withDefaultValues();

    $tcps->setBaggage($baggage);
    expect($tcps->getBaggage()?->trace_context)->toBeNull();

    $baggage = ContextProp\Baggage::fromShape(shape(
      'trace_context' => ContextProp\TraceContext::withDefaultValues(),
    ));
    $tcps->setBaggage($baggage);
    expect($tcps->getBaggage()?->trace_context)->toNotBeNull();
  }

  public function testPrivacyUniverse(): void {
    $tcps = ThriftContextPropState::get();
    expect($tcps->getPrivacyUniverseDesignator())->toBeNull();
    $tcps->setPrivacyUniverse(123);
    expect($tcps->getPrivacyUniverseDesignator()?->getValue())->toNotBeNullAnd()
      ->toEqual(123);
  }

  public function testInitialization()[defaults]: void {
    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->request_id = "13579";

    $buf = new TMemoryBuffer();
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm->write($prot);
    $s = $buf->getBuffer();
    $e = Base64::encode($s);

    ThriftContextPropState::initFromString($e);
    $tcps = ThriftContextPropState::get();
    expect($tcps->getRequestId())->toEqual("13579");
  }

  public async function testInitializationWithUserIds(
  )[defaults]: Awaitable<void> {
    self::mockFunction(IgidUtils::isUserFbid<>)->mockReturn(true);
    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->baggage = ContextProp\Baggage::withDefaultValues();
    $tfm->baggage->user_ids = ContextProp\UserIds::fromShape(
      shape('fb_user_id' => 123, 'ig_user_id' => 456),
    );

    $buf = new TMemoryBuffer();
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm->write($prot);
    $s = $buf->getBuffer();
    $e = Base64::encode($s);

    ThriftContextPropState::initFromString($e);
    $tcps = ThriftContextPropState::get();
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(123);
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(456);
  }

  public async function testInitializationWithVC()[defaults]: Awaitable<void> {
    MockJustKnobs::setBool('meta_cp/www:enable_user_id_ctx_prop', true);
    self::mockFunction(IgidUtils::isUserFbid<>)->mockReturn(true);
    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->baggage = ContextProp\Baggage::withDefaultValues();
    $tfm->baggage->user_ids = ContextProp\UserIds::fromShape(
      shape('fb_user_id' => null, 'ig_user_id' => null),
    );

    // set up mocks for VC
    $fb_vc = mock(IFBViewerContext::class)->mockReturn('getAccountID', fbid(1));
    $ig_vc = mock(IIGViewerContext::class)->mockReturn('getViewerID', fbid(2));

    $buf = new TMemoryBuffer();
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm->write($prot);
    $s = $buf->getBuffer();
    $e = Base64::encode($s);

    ThriftContextPropState::initFromString($e);
    ThriftContextPropState::updateUserIdFromVC($fb_vc, "test");
    ThriftContextPropState::updateUserIdFromVC($ig_vc, "test");

    $tcps = ThriftContextPropState::get();
    // expect user ids were set from fetched values
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(1);
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(2);
  }

  public async function testInitializationWithBothTFMandVC(
  )[defaults]: Awaitable<void> {
    MockJustKnobs::setBool('meta_cp/www:enable_user_id_ctx_prop', true);
    self::mockFunction(IgidUtils::isUserFbid<>)->mockReturn(true);
    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->baggage = ContextProp\Baggage::withDefaultValues();
    $tfm->baggage->user_ids = ContextProp\UserIds::fromShape(
      shape('fb_user_id' => null, 'ig_user_id' => 456),
    );

    // set up mocks for VC
    $fb_vc =
      mock(IFBViewerContext::class)->mockReturn('getAccountID', fbid(123));

    $buf = new TMemoryBuffer();
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm->write($prot);
    $s = $buf->getBuffer();
    $e = Base64::encode($s);

    ThriftContextPropState::initFromString($e);
    ThriftContextPropState::updateUserIdFromVC($fb_vc, "test");

    $tcps = ThriftContextPropState::get();
    // expect fb user id to be populated
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(123);
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(456);
  }

  public async function testInitializationUserIdsNotOverwritten(
  )[defaults]: Awaitable<void> {
    MockJustKnobs::setBool('meta_cp/www:enable_user_id_ctx_prop', true);
    self::mockFunction(IgidUtils::isUserFbid<>)->mockReturn(true);
    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->baggage = ContextProp\Baggage::withDefaultValues();
    $tfm->baggage->user_ids = ContextProp\UserIds::fromShape(
      shape('fb_user_id' => 123, 'ig_user_id' => 456),
    );
    // set up mocks for VC
    $fb_vc = mock(IFBViewerContext::class)->mockReturn('getAccountID', fbid(1));
    $ig_vc = mock(IIGViewerContext::class)->mockReturn('getViewerID', fbid(2));

    $buf = new TMemoryBuffer();
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm->write($prot);
    $s = $buf->getBuffer();
    $e = Base64::encode($s);

    ThriftContextPropState::initFromString($e);
    ThriftContextPropState::updateUserIdFromVC($fb_vc, "test");
    ThriftContextPropState::updateUserIdFromVC($ig_vc, "test");

    $tcps = ThriftContextPropState::get();
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(1);
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(2);
  }

  public async function testUpdatedWithExplicitFBUserId(
  )[defaults]: Awaitable<void> {
    MockJustKnobs::setBool('meta_cp/www:enable_user_id_ctx_prop', true);
    self::mockFunction(IgidUtils::isUserFbid<>)->mockReturn(true);
    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->baggage = ContextProp\Baggage::withDefaultValues();
    $tfm->baggage->user_ids = ContextProp\UserIds::fromShape(
      shape('fb_user_id' => null, 'ig_user_id' => 456),
    );

    $buf = new TMemoryBuffer();
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm->write($prot);
    $s = $buf->getBuffer();
    $e = Base64::encode($s);

    ThriftContextPropState::initFromString($e);
    // expect these to be no-op if TFM already has user ids
    ThriftContextPropState::updateFBUserId(
      1,
      "test",
      UserIdSource::VIEWER_CONTEXT,
    );

    $tcps = ThriftContextPropState::get();
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(1);
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(456);
  }

  public async function testUpdatedWithExplicitFBUserIdNoOverwrite(
  )[defaults]: Awaitable<void> {
    MockJustKnobs::setBool('meta_cp/www:enable_user_id_ctx_prop', true);
    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->baggage = ContextProp\Baggage::withDefaultValues();
    $tfm->baggage->user_ids = ContextProp\UserIds::fromShape(
      shape('fb_user_id' => 123, 'ig_user_id' => null),
    );

    $buf = new TMemoryBuffer();
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm->write($prot);
    $s = $buf->getBuffer();
    $e = Base64::encode($s);

    ThriftContextPropState::initFromString($e);
    ThriftContextPropState::updateFBUserId(
      456,
      "test",
      UserIdSource::VIEWER_CONTEXT,
    );

    $tcps = ThriftContextPropState::get();
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(456);
  }

  public async function testUpdatedWithExplicitIGUserId(
  )[defaults]: Awaitable<void> {
    MockJustKnobs::setBool('meta_cp/www:enable_user_id_ctx_prop', true);
    self::mockFunction(IgidUtils::isUserFbid<>)->mockReturn(true);
    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->baggage = ContextProp\Baggage::withDefaultValues();
    $tfm->baggage->user_ids = ContextProp\UserIds::fromShape(
      shape('fb_user_id' => 456, 'ig_user_id' => null),
    );

    $buf = new TMemoryBuffer();
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm->write($prot);
    $s = $buf->getBuffer();
    $e = Base64::encode($s);

    ThriftContextPropState::initFromString($e);
    ThriftContextPropState::updateIGUserId(
      1,
      "test",
      UserIdSource::VIEWER_CONTEXT,
    );

    $tcps = ThriftContextPropState::get();
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(456);
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(1);
  }

  public async function testUpdatedWithExplicitIGUserIdNoOverwrite(
  )[defaults]: Awaitable<void> {
    MockJustKnobs::setBool('meta_cp/www:enable_user_id_ctx_prop', true);
    self::mockFunction(IgidUtils::isUserFbid<>)->mockReturn(true);
    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->baggage = ContextProp\Baggage::withDefaultValues();
    $tfm->baggage->user_ids = ContextProp\UserIds::fromShape(
      shape('fb_user_id' => null, 'ig_user_id' => 123),
    );

    $buf = new TMemoryBuffer();
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm->write($prot);
    $s = $buf->getBuffer();
    $e = Base64::encode($s);

    ThriftContextPropState::initFromString($e);
    ThriftContextPropState::updateIGUserId(
      456,
      "test",
      UserIdSource::VIEWER_CONTEXT,
    );

    $tcps = ThriftContextPropState::get();
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(456);
  }

  public function testGen()[defaults]: void {
    ThriftContextPropState::initFromString(null);
    $tcps = ThriftContextPropState::get();
    $rid = $tcps->getRequestId();
    expect($rid)->toNotBeNull();
    expect(Str\length($rid))->toEqual(16);
  }

  public function testEx()[defaults]: void {
    $garbage = "abcdefg";
    ThriftContextPropState::initFromString($garbage);
    // No exception is thrown
    // RequestId is initialized
    $tcps = ThriftContextPropState::get();
    $rid = $tcps->getRequestId();
    expect($rid)->toNotBeNull();
    expect(Str\length($rid))->toEqual(16);
  }

  public function testRequestPriorityNullable(): void {
    $tcps = ThriftContextPropState::get();
    expect($tcps->getRequestPriority())->toBeNull();
    $tcps->setRequestPriority(RequestPriority::CRITICAL);
    expect($tcps->getRequestPriority())->toEqual(RequestPriority::CRITICAL);
    $tcps->setRequestPriority(null);
    expect($tcps->getRequestPriority())->toBeNull();
    $tcps->setRequestPriority(RequestPriority::SHEDDABLE);
    expect($tcps->getRequestPriority())->toEqual(RequestPriority::SHEDDABLE);
    $tcps->setRequestPriority(null);
    expect($tcps->getRequestPriority())->toBeNull();
  }

  public function testDirtying()[defaults]: void {
    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->request_id = "13579";
    $tfm->origin_id = 54321;

    $buf = new TMemoryBuffer();
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm->write($prot);
    $s = $buf->getBuffer();
    $e = Base64::encode($s);

    ThriftContextPropState::initFromString($e);
    $tcps = ThriftContextPropState::get();
    $serialized_before = $tcps->getSerialized();
    MCPContext::setGlobal__UNSAFE(MCPOriginIDs::INTEGRITY);
    $serialized_after = $tcps->getSerialized();
    expect($serialized_before)->toNotEqual($serialized_after);
  }

  public function testModelInfo(): void {
    $tcps = ThriftContextPropState::get();
    $tcps->clear();
    expect($tcps->getBaggage())->toBeNull();
    expect($tcps->getModelInfo())->toBeNull();
    expect($tcps->getModelTypeId())->toBeNull();

    $baggage = ContextProp\Baggage::withDefaultValues();
    $tcps->setBaggage($baggage);
    expect($tcps->getModelInfo())->toBeNull();
    expect($tcps->getModelTypeId())->toBeNull();

    $baggage = ContextProp\Baggage::fromShape(shape(
      'model_info' => ContextProp\ModelInfo::withDefaultValues(),
    ));
    $tcps->setBaggage($baggage);
    expect($tcps->getModelInfo())->toNotBeNull();
    expect($tcps->getModelTypeId())->toBeNull();

    $baggage = ContextProp\Baggage::fromShape(shape(
      'model_info' =>
        ContextProp\ModelInfo::fromShape(shape('model_type_id' => 12345)),
    ));
    $tcps->setBaggage($baggage);
    expect($tcps->getModelInfo()?->get_model_type_id())->toEqual(12345);
    expect($tcps->getModelTypeId())->toEqual(12345);
  }

  public function testBaggageFlags1(): void {
    $tcps = ThriftContextPropState::get();
    $tcps->clear();
    expect($tcps->getBaggageFlags1())->toBeNull();

    $tcps->setBaggageFlags1(11);
    expect($tcps->getBaggageFlags1())->toEqual(11);
    // override existing value
    $tcps->setBaggageFlags1(12);
    expect($tcps->getBaggageFlags1())->toEqual(12);
  }

  private static function setupBaggageFlags1(
    Set<ContextProp\BaggageFlags1> $flags,
  ): void {
    ThriftContextPropState::get()->clear();
    $flags1 = 0;
    foreach ($flags as $flag) {
      $flags1 |= (1 << (int)$flag);
    }
    ThriftContextPropState::get()->setBaggageFlags1($flags1);
  }

  public async function testIsBaggageFlags1Set(): Awaitable<void> {
    ThriftContextPropStateTest::setupBaggageFlags1(Set {});
    $tcps = ThriftContextPropState::get();
    expect($tcps->isBaggageFlags1Set(ContextProp\BaggageFlags1::IS_EXPERIMENT))
      ->toBeFalse();

    ThriftContextPropStateTest::setupBaggageFlags1(
      Set {ContextProp\BaggageFlags1::IS_EXPERIMENT},
    );
    $tcps = ThriftContextPropState::get();
    expect($tcps->isBaggageFlags1Set(ContextProp\BaggageFlags1::IS_EXPERIMENT))
      ->toBeTrue();
  }

  public async function testSetBaggageFlags1ByName(): Awaitable<void> {
    ThriftContextPropStateTest::setupBaggageFlags1(Set {});
    $tcps = ThriftContextPropState::get();
    expect($tcps->isBaggageFlags1Set(ContextProp\BaggageFlags1::IS_EXPERIMENT))
      ->toBeFalse();

    $tcps->setBaggageFlags1ByName(ContextProp\BaggageFlags1::IS_EXPERIMENT);
    expect($tcps->isBaggageFlags1Set(ContextProp\BaggageFlags1::IS_EXPERIMENT))
      ->toBeTrue();
    expect($tcps->isBaggageFlags1Set(ContextProp\BaggageFlags1::NOT_SET))
      ->toBeFalse();

    // setting IS_EXPERIMENT again is passive
    $tcps->setBaggageFlags1ByName(ContextProp\BaggageFlags1::IS_EXPERIMENT);
    expect($tcps->isBaggageFlags1Set(ContextProp\BaggageFlags1::IS_EXPERIMENT))
      ->toBeTrue();

    // setting NOT_ALLOWED is passive
    $tcps->setBaggageFlags1ByName(ContextProp\BaggageFlags1::NOT_ALLOWED);
    expect($tcps->isBaggageFlags1Set(ContextProp\BaggageFlags1::NOT_ALLOWED))
      ->toBeFalse();
  }

  public async function testClearBaggageFlags1ByName(): Awaitable<void> {
    ThriftContextPropStateTest::setupBaggageFlags1(
      Set {ContextProp\BaggageFlags1::IS_EXPERIMENT},
    );
    $tcps = ThriftContextPropState::get();
    expect($tcps->isBaggageFlags1Set(ContextProp\BaggageFlags1::IS_EXPERIMENT))
      ->toBeTrue();

    $tcps->clearBaggageFlags1ByName(ContextProp\BaggageFlags1::IS_EXPERIMENT);
    expect($tcps->isBaggageFlags1Set(ContextProp\BaggageFlags1::IS_EXPERIMENT))
      ->toBeFalse();

    // clearing IS_EXPERIMENT again is passive
    $tcps->clearBaggageFlags1ByName(ContextProp\BaggageFlags1::IS_EXPERIMENT);
    expect($tcps->isBaggageFlags1Set(ContextProp\BaggageFlags1::IS_EXPERIMENT))
      ->toBeFalse();

    // clearing NOT_ALLOWED is passive
    $tcps->clearBaggageFlags1ByName(ContextProp\BaggageFlags1::NOT_ALLOWED);
    expect($tcps->isBaggageFlags1Set(ContextProp\BaggageFlags1::NOT_ALLOWED))
      ->toBeFalse();
  }

  public function testBaggageRootProductId(): void {
    $tcps_with_empty_baggage = ThriftContextPropState::get();
    $tcps_with_empty_baggage->clear();
    expect($tcps_with_empty_baggage->getBaggage())->toBeNull();
    expect(readonly $tcps_with_empty_baggage->getRootProductId())->toBeNull();

    $tcps = ThriftContextPropState::get();
    expect(readonly $tcps->getRootProductId())->toBeNull();

    $root_product_id = $tcps->setRootProductId(789);
    expect(readonly $tcps->getRootProductId())->toEqual(789);
    expect($root_product_id)->toEqual(789);

    // Test overrding existing value should not be allowed
    $root_product_id = $tcps->setRootProductId(100);
    expect(readonly $tcps->getRootProductId())->toEqual(789);
    expect($root_product_id)->toEqual(789);
  }

  public function testDisableIngestingExperimentIds(): void {
    // Arrange
    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->experiment_ids = vec[1, 2, 3];
    $buf = new TMemoryBuffer();
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm->write($prot);
    $s = $buf->getBuffer();
    $e = Base64::encode($s);

    // Act
    ThriftContextPropState::initFromString($e, true);

    // Assert
    $tcps = ThriftContextPropState::get();
    expect($tcps->getExperimentIds())->toBeEmpty();
  }

  public function testRequestIdEncoded(): void {
    $tcps = ThriftContextPropState::get();
    $tcps->setRequestId("\x00");
    expect($tcps->getRequestIdEncoded())->toEqual("AA==");
  }

  public function testRequestIdEncodedEmpty(): void {
    $tcps = ThriftContextPropState::get();
    $tcps->setRequestId("");
    expect($tcps->getRequestIdEncoded())->toEqual("0");
  }

  public function testTraceSizeEstimationBreadth(): void {
    $tcps = ThriftContextPropState::get();
    $tcps->clear();
    expect($tcps->getBreadth())->toBeNull();

    // Set breadth depth product
    $tcps->setBreadth(100);
    expect($tcps->getBreadth())->toEqual(100);

    // Override existing value
    $tcps->setBreadth(200);
    expect($tcps->getBreadth())->toEqual(200);

    // Verify it's properly nested in the baggage structure
    expect($tcps->getBaggage())->toNotBeNull();
    expect($tcps->getTraceContext())->toNotBeNull();
    expect($tcps->getTraceContext()?->tracing_context)->toNotBeNull();
    expect($tcps->getTraceContext()?->tracing_context?->estimate)
      ->toNotBeNull();
  }

  public function testTraceSizeEstimationDepth(): void {
    $tcps = ThriftContextPropState::get();
    $tcps->clear();
    expect($tcps->getDepth())->toBeNull();

    // Set depth
    $tcps->setDepth(10);
    expect($tcps->getDepth())->toEqual(10);

    // Override existing value
    $tcps->setDepth(20);
    expect($tcps->getDepth())->toEqual(20);

    // Verify it's properly nested in the baggage structure
    expect($tcps->getBaggage())->toNotBeNull();
    expect($tcps->getTraceContext())->toNotBeNull();
    expect($tcps->getTraceContext()?->tracing_context)->toNotBeNull();
    expect($tcps->getTraceContext()?->tracing_context?->estimate)
      ->toNotBeNull();
  }

  public function testTraceSizeEstimationBothFields(): void {
    $tcps = ThriftContextPropState::get();
    $tcps->clear();

    // Set both fields
    $tcps->setBreadth(500);
    $tcps->setDepth(25);

    // Verify both fields are set correctly
    expect($tcps->getBreadth())->toEqual(500);
    expect($tcps->getDepth())->toEqual(25);

    // Verify the estimate struct exists
    $estimate = $tcps->getTraceContext()?->tracing_context?->estimate;
    expect($estimate)->toNotBeNull();
    expect($estimate?->breadth)->toEqual(500);
    expect($estimate?->depth)->toEqual(25);
  }

  public function testTraceSizeEstimationDirtiesCache(): void {
    $tcps = ThriftContextPropState::get();
    $tcps->clear();

    // Serialize the initial state to populate the cache
    $serialized_before = $tcps->getSerialized();

    // Set breadth depth product, which should dirty the cache
    $tcps->setBreadth(300);

    // Serialize the state again
    $serialized_after = $tcps->getSerialized();

    // The serialized strings should be different because the cache was dirtied
    expect($serialized_before)->toNotEqual($serialized_after);
  }

}

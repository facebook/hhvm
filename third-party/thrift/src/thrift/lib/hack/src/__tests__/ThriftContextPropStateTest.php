<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift')>>
final class ThriftContextPropStateTest extends WWWTest {
  use ClassLevelTest;
  public function testAccess(): void {
    $tcps = ThriftContextPropState::get();
    expect($tcps->getRequestId())->toEqual("");
    $tcps->setRequestId("12345");
    expect(ThriftContextPropState::get()->getRequestId())->toEqual("12345");
  }

  public function testOriginIdNullable(): void {
    $tcps = ThriftContextPropState::get();
    expect($tcps->getOriginId())->toBeNull();
    // 0 is different from null
    $tcps->setOriginId(0);
    expect($tcps->getOriginId())->toEqual(0);
    $tcps->setOriginId(null);
    expect($tcps->getOriginId())->toBeNull();
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
    $tcps = ThriftContextPropState::get();
    expect($tcps->getUserIds())->toBeNull();

    // 0 is different from null
    $tcps->setUserIds(ContextProp\UserIds::withDefaultValues());
    expect($tcps->getUserIds())->toNotBeNull();
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(0);
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(0);

    // override existing value
    $tcps->setUserIds(new ContextProp\UserIds(1, 2));
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(1);
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(2);

    // back to null
    $tcps->setUserIds(null);
    expect($tcps->getUserIds())->toBeNull();

    // set FB Id only
    $tcps->setFBUserId(3);
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(3);
    expect($tcps->getFBUserId())->toEqual(3);
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(0);

    // set IG Id only
    $tcps->setIGUserId(4);
    expect($tcps->getUserIds()?->fb_user_id)->toEqual(3);
    expect($tcps->getUserIds()?->ig_user_id)->toEqual(4);
    expect($tcps->getIGUserId())->toEqual(4);

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
    expect($tcps->getPrivacyUniverse())->toBeNull();
    $tcps->setPrivacyUniverse(123);
    expect($tcps->getPrivacyUniverse())->toEqual(123);
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
    $tcps->setOriginId(12345);
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

}

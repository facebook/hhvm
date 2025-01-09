<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift')>>
final class ThriftContextPropHandlerTest extends WWWTest {
  use ClassLevelTest;
  private static function readTFMForTest(string $v): ThriftFrameworkMetadata {
    $transport = Base64::decode($v);
    $buf = new TMemoryBuffer($transport);
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->read($prot);
    return $tfm;
  }

  public function testValue(): void {
    ThriftContextPropState::get()->setRequestId("abcba");
    ThriftContextPropState::get()->setOriginId(777);
    $v = ThriftContextPropHandler::makeV();
    if ($v !== null) {
      $transport = Base64::decode($v);
      $buf = new TMemoryBuffer($transport);
      $prot = new TCompactProtocolAccelerated($buf);
      $tfm = ThriftFrameworkMetadata::withDefaultValues();
      expect($tfm->request_id)->toBeNull();
      $tfm->read($prot);
      expect($tfm->request_id)->toEqual("abcba");
      expect($tfm->origin_id)->toEqual(777);
    } else {
      expect(true)->toBeFalse();
    }
  }

  public function testValueWithUniverse(): void {
    ThriftContextPropState::get()->setPrivacyUniverse(1234);
    $v = ThriftContextPropHandler::makeV();
    if ($v is null) {
      expect($v)->toNotBeNull();
      return;
    }

    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->read(
      new TCompactProtocolAccelerated(new TMemoryBuffer(Base64::decode($v))),
    );
    expect($tfm->privacyUniverse)->toEqual(1234);
  }

  public function testValueNull(): void {
    ThriftContextPropState::get()->setRequestId(null);
    ThriftContextPropState::get()->setOriginId(null);
    ThriftContextPropState::get()->setPrivacyUniverse(null);
    $v = ThriftContextPropHandler::makeV();
    expect($v)->toBeNull();

    ThriftContextPropState::get()->setRequestId("1234");
    $v = ThriftContextPropHandler::makeV();
    expect($v)->toNotBeNull();

    ThriftContextPropState::get()->setRequestId(null);
    $v = ThriftContextPropHandler::makeV();
    expect($v)->toBeNull();

    ThriftContextPropState::get()->setOriginId(0);
    $v = ThriftContextPropHandler::makeV();
    expect($v)->toNotBeNull();

    ThriftContextPropState::get()->setOriginId(null);
    ThriftContextPropState::get()->setPrivacyUniverse(123);
    $v = ThriftContextPropHandler::makeV();
    expect($v)->toNotBeNull();
  }
}

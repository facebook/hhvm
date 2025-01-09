<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('signals_infra')>>
final class ThriftImmutableWrapperTest extends WWWTest {

  use ClassLevelTest;

  public async function testGetSerializedThrift(): Awaitable<void> {
    $underlying = AdsConsentPlatformParams::fromShape(shape(
      'traceID' => '123',
    ));

    $test = new TestThriftImmutableWrapper($underlying);

    $test_serialized =
      $test->serialize(SignalsPipeSerializationProtocol::BINARY);

    $test_deserialized = SignalsPipeUtils::deserializeThrift(
      AdsConsentPlatformParams::withDefaultValues(),
      $test_serialized,
      SignalsPipeSerializationProtocol::BINARY,
    );
    expect($test_deserialized)->toBePHPEqual($underlying);
    expect($test_deserialized?->__toShape())->toBePHPEqual(
      $underlying->__toShape(),
    );

  }

  public async function testCreateDeepCopy(): Awaitable<void> {
    $underlying = AdsConsentPlatformParams::fromShape(shape(
      'traceID' => '123',
    ));

    $test = new TestThriftImmutableWrapper($underlying);

    $copy = $test->createDeepCopy();
    expect($copy)->toNotEqual($test);
    expect($test->getTraceId())->toEqual('123');
    expect($copy->traceID)->toEqual('123');

    $underlying->traceID = '456';
    expect($test->getTraceId())->toEqual('456');
    expect($copy->traceID)->toEqual('123');

    $copy->traceID = '789';
    expect($test->getTraceId())->toEqual('456');
    expect($copy->traceID)->toEqual('789');
  }

  public async function testToString(): Awaitable<void> {
    $underlying = AdsConsentPlatformParams::fromShape(shape(
      'traceID' => '123',
    ));

    $test = new TestThriftImmutableWrapper($underlying);

    $wrapper_string = $test->toString();
    $default_thrift = JSON::encode($underlying);
    $default_wrapper = JSON::encode($test);
    expect($wrapper_string)->toEqual($default_thrift);
    expect($wrapper_string)->toEqual($default_wrapper);
  }

  public async function testInstanceKey(): Awaitable<void> {
    $underlying = AdsConsentPlatformParams::fromShape(shape(
      'traceID' => '123',
    ));

    $test = new TestThriftImmutableWrapper($underlying);

    $key = $test->getInstanceKey();
    $thrift_key = $underlying->getInstanceKey();
    expect($key)->toEqual($thrift_key);
  }
}

final class TestThriftImmutableWrapper
  extends ThriftImmutableWrapper<AdsConsentPlatformParams> {

  public function getTraceId(): ?string {
    return $this->data->traceID;
  }

}

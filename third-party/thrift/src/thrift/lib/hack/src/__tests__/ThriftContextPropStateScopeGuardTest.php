<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('xdc_artillery')>>
final class ThriftContextPropStateScopeGuardTest extends WWWTest {

  private function getSerializedTFMHeaders(
    string $overriden_request_id,
    int $overriden_origin_id,
  ): dict<string, string> {
    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $tfm->request_id = $overriden_request_id;
    $tfm->origin_id = $overriden_origin_id;

    $buf = new TMemoryBuffer();
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm->write($prot);
    $s = $buf->getBuffer();
    $e = Base64::encode($s);
    $serialized_headers = dict[
      ThriftFrameworkMetadata_CONSTANTS::ThriftFrameworkMetadataHeaderKey => $e,
    ];
    return $serialized_headers;
  }

  public function testInitFromSerializedContext()[defaults]: void {
    $original_request_id = "9999";
    $original_origin_id = 12345;
    $overriden_request_id_0 = "13579";
    $overriden_origin_id_0 = 54321;
    $overriden_request_id_1 = "00000";
    $overriden_origin_id_1 = 89765;

    ThriftContextPropState::get()->setRequestId($original_request_id);
    ThriftContextPropState::get()->setOriginId($original_origin_id);

    expect(ThriftContextPropState::get()->getRequestId())->toEqual(
      $original_request_id,
    );
    expect(ThriftContextPropState::get()->getOriginId())->toEqual(
      $original_origin_id,
    );
    $serialized_headers_0 = $this->getSerializedTFMHeaders(
      $overriden_request_id_0,
      $overriden_origin_id_0,
    );
    $serialized_headers_1 = $this->getSerializedTFMHeaders(
      $overriden_request_id_1,
      $overriden_origin_id_1,
    );
    using (ThriftContextPropStateScopeGuard::create($serialized_headers_0)) {
      expect(ThriftContextPropState::get()->getRequestId())->toEqual(
        $overriden_request_id_0,
      );
      expect(ThriftContextPropState::get()->getOriginId())->toEqual(
        $overriden_origin_id_0,
      );
      using (ThriftContextPropStateScopeGuard::create($serialized_headers_1)) {
        expect(ThriftContextPropState::get()->getRequestId())->toEqual(
          $overriden_request_id_1,
        );
        expect(ThriftContextPropState::get()->getOriginId())->toEqual(
          $overriden_origin_id_1,
        );
      }

    }

    expect(ThriftContextPropState::get()->getRequestId())->toEqual(
      $original_request_id,
    );
    expect(ThriftContextPropState::get()->getOriginId())->toEqual(
      $original_origin_id,
    );

  }

  public function testInitFromSerializedContextDontThrowWhenTFMNotFound(
  )[defaults]: void {

    $original_request_id = ThriftContextPropState::get()->getRequestId();
    $original_origin_id = ThriftContextPropState::get()->getOriginId();
    $serialized_context = dict[
      "GarbageKey" => "GarbageValue",
    ];
    {
      using (ThriftContextPropStateScopeGuard::create($serialized_context)) {
        // No exception is thrown
        // RequestId is initialized
        // Origin Id is null
        expect(ThriftContextPropState::get()->getRequestId())->toNotBeNull();
        expect(ThriftContextPropState::get()->getOriginId())->toBeNull();
      }

    }

    using (ThriftContextPropStateScopeGuard::create($serialized_context)) {
      // No exception is thrown
      // RequestId is initialized
      // Origin Id is null
      expect(ThriftContextPropState::get()->getRequestId())->toNotBeNull();
      expect(ThriftContextPropState::get()->getOriginId())->toBeNull();
    }

    expect(ThriftContextPropState::get()->getRequestId())->toEqual(
      $original_request_id,
    );
    expect(ThriftContextPropState::get()->getOriginId())->toEqual(
      $original_origin_id,
    );

  }
}

<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

/**
 * ThriftContextPropScopeGuard allows consumers to override ThriftContextPropState within the scope and restore the previous state upon disposal.
 */
final class ThriftContextPropStateScopeGuard {
  private ?string $prevSerializedThriftContextPropState;

  <<__ReturnDisposable>>
  public static function create(
    dict<string, string> $serialized_headers,
  ): ThriftContextPropStateScopeGuard {
    return new ThriftContextPropStateScopeGuard($serialized_headers);
  }

  private function __construct(dict<string, string> $serialized_headers) {

    $this->prevSerializedThriftContextPropState =
      ThriftContextPropState::get()->getSerialized();
    ThriftContextPropState::get()->clear();
    ThriftContextPropState::initFromString(idx(
      $serialized_headers,
      ThriftFrameworkMetadata_CONSTANTS::ThriftFrameworkMetadataHeaderKey,
    ));
  }

  public function __dispose(): void {
    ThriftContextPropState::initFromString(
      $this->prevSerializedThriftContextPropState,
    );
  }

}

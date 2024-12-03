<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

final class UpdateUniverseContextHandler implements IContextHandler {
  use TThriftPoliciedOptOutList;

  public function onIncomingDownstream(
    ThriftContextPropState $mutable_ctx,
    ClientInstrumentationParams $params,
    ImmutableThriftFrameworkMetadataOnResponse $immutable_tfmr,
  ): void {}

  public function onIncomingUpstream(
    ThriftContextPropState $mutable_ctx,
    ServerInstrumentationParams $params,
    ImmutableThriftFrameworkMetadata $immutable_tfm,
  ): void {}

  public function onOutgoingUpstream(
    ServerInstrumentationParams $params,
    ThriftFrameworkMetadataOnResponse $mutable_tfmr,
    ImmutableThriftContextPropState $immutable_ctx,
  ): void {}

  public function onOutgoingDownstream(
    ClientInstrumentationParams $params,
    ThriftFrameworkMetadata $mutable_tfm,
    ImmutableThriftContextPropState $immutable_ctx,
  ): void {
    $raw_thrift_class_name = Shapes::idx($params, 'thrift_class');
    $client = Shapes::idx($params, 'client');
    $function_name = Shapes::idx($params, 'fn_name');
    if (
      $raw_thrift_class_name is null ||
      $function_name is null ||
      $client is null
    ) {
      return;
    }

    $thrift_name = HH\class_to_classname($raw_thrift_class_name);

    $thrift_name =
      ThriftContextPropUpdateUniverseHandler::getTransformedThriftClientName(
        $thrift_name,
        $client,
      );

    if (self::isServiceNameOptedOut($thrift_name)) {
      return;
    }

    self::updateContextPropUniverseInThriftFrameworkMetadata(
      $thrift_name,
      $function_name,
      $mutable_tfm,
    );
  }

  public static function updateContextPropUniverseInThriftFrameworkMetadata(
    string $thrift_name,
    string $function_name,
    ThriftFrameworkMetadata $mutable_tfm,
  )[zoned_local]: void {
    if (PrivacyLibKS::isKilled(PLKS::XSU_UNIVERSE_CONTEXT_PROP)) {
      return;
    }

    try {
      $current_universe =
        ThriftContextPropUpdateUniverseHandler::getCurrentUniverse(
          $thrift_name,
          $function_name,
        );
      // set current universe in TFM
      $current_universe_int = $current_universe?->getValue();
      if (
        $current_universe_int is nonnull &&
        $mutable_tfm->privacyUniverse !== $current_universe_int
      ) {
        $mutable_tfm->privacyUniverse = $current_universe_int;
      }
    } catch (Exception $e) {
      FBLogger('privacylib', 'thrift_propagation_exception')
        ->handle(
          $e,
          Causes::the('Universe')->to('not update')
            ->document('fail to update thrift context prop universe'),
        );
    }
  }
}

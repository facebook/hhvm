<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('www_privacy_frameworks')>>
final class ThriftServiceMethodNameVirtualPolicyEnforcer
  implements IPolicyEnforcerAtThrift {
  const type TPrivacyLib = ThriftServiceMethodNamePrivacyLib;
  const PolicyEnforcerAssetClass ASSET_CLASS =
    PolicyEnforcerAssetClass::THRIFT_CLIENT;

  public static function getCallerName(
    string $asset_type, // This is actually the service name in this context
    ?int $caller,
  )[]: ?string {
    switch ($asset_type) {
      case 'CachiusService':
        $caller_enum = CachiusCaller::class;
        break;
      case 'LaserLeaf':
        $caller_enum = LaserCaller::class;
        break;
      case 'TitanMessagingService':
        $caller_enum = CallistoCaller::class;
        break;
      case 'AdalService':
        $caller_enum = AdalMySQLThriftCaller::class;
        break;
      default:
        return null;
    }
    return idx($caller_enum::getNames(), $caller_enum::coerce($caller));
  }

  // TODO(duan): special usecase for this policyEnforcer
  public static function isConditionallyAllowed(
    PolicyEnforcerCallerIdentity $caller,
    string $asset_type,
  )[leak_safe]: bool {
    return ThriftPolicyEnforcer::isConditionallyAllowed($caller, $asset_type);
  }

  public static function extractEnforcableAssets(
    string $func_name,
    ?IThriftStruct $args,
    ?string $smc_service_name,
  )[leak_safe]: keyset<string> {
    // does not matter, thrift always return service name as enforcable asset
    // see TClientPoliciedAsyncHandler::extractPolicyEnforcerClsAndAssets for more details
    return keyset[];
  }

  public static async function genEnforcePolicyEnforcerAndPolicyZone(
    string $asset_type,
    string $policy_enforcer_api,
    PolicyEnforcerCallerIdentity $caller,
    PolicyEnforcerContext $context,
  )[zoned_shallow]: Awaitable<TPolicyEnforcerResult> {
    $old_exception = null;
    $old_result = PolicyEnforcer::DEFAULT_POLICY_ENFORCER_RESULT;

    try {
      $old_result = await self::genPolicyEnforcerResults(
        $asset_type,
        $policy_enforcer_api,
        $caller,
        $context,
      );
    } catch (Exception $e) {
      $old_exception = $e;
    }

    if ($old_exception is nonnull) {
      throw $old_exception;
    }

    // We only run probes if no exception is thrown
    // Pending Probes PrivacyLib Integration after PAAL Consolidation
    if (!PrivacyLibKS::isKilled(PLKS::TIR_THRIFT_PROBES)) {
      await self::genExecuteStandaloneProbes(
        $asset_type,
        $policy_enforcer_api,
        $caller,
        $context,
      );
    }

    return $old_result;
  }

  private static async function genPolicyEnforcerResults(
    string $asset_type,
    string $policy_enforcer_api,
    PolicyEnforcerCallerIdentity $caller,
    PolicyEnforcerContext $context,
  )[zoned_local]: Awaitable<TPolicyEnforcerResult> {
    // This may throw exceptions
    return await ThriftPolicyEnforcer::genEnforcePolicyEnforcerAndPolicyZone(
      $asset_type,
      $policy_enforcer_api,
      $caller,
      $context,
    );
  }

  public static async function genProcessResponse(
    string $asset_type,
    string $policy_enforcer_api,
    PolicyEnforcerCallerIdentity $caller,
    PolicyEnforcerContext $context,
  ): Awaitable<void> {
    // Pending Probes PrivacyLib post-read Integration
    if (!PrivacyLibKS::isKilled(PLKS::TIR_THRIFT_PROBES)) {
      await self::genExecuteStandaloneProbes(
        $asset_type,
        $policy_enforcer_api,
        $caller,
        $context,
      );
    } else {
      await ThriftPolicyEnforcer::genProcessResponse(
        $asset_type,
        $policy_enforcer_api,
        $caller,
        $context,
      );
    }
  }

  private static async function genExecuteStandaloneProbes(
    string $asset_type,
    string $policy_enforcer_api,
    PolicyEnforcerCallerIdentity $caller,
    PolicyEnforcerContext $context,
  )[zoned_local]: Awaitable<void> {
    await PrivacyProbesThriftModule::genExecuteStandaloneProbes(
      shape(
        'asset_class' => static::ASSET_CLASS,
        'asset_type_config' => shape(
          'asset_type' => $asset_type,
          'config' => null, // does not matter for probes
        ),
        'api' => $policy_enforcer_api, // does not matter for probes
        'caller' => $caller, // does not matter for probes
      ),
      $context,
    );
  }
}

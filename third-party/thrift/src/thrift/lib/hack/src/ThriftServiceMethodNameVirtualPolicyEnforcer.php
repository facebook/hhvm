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
    // Pending PAAL Consolidation
    return await self::genPolicyEnforcerResults(
      $asset_type,
      $policy_enforcer_api,
      $caller,
      $context,
    );

  }

  private static async function genPolicyEnforcerResults(
    string $asset_type,
    string $policy_enforcer_api,
    PolicyEnforcerCallerIdentity $caller,
    PolicyEnforcerContext $context,
  )[zoned_local]: Awaitable<TPolicyEnforcerResult> {
    // This may throw exceptions
    $result = await ThriftPolicyEnforcer::genEnforcePolicyEnforcerAndPolicyZone(
      $asset_type,
      $policy_enforcer_api,
      $caller,
      $context,
    );

    // Pending Probes PrivacyLib post-rpc Integration
    if (!PrivacyLibKS::isKilled(PLKS::TIR_THRIFT_PROBES)) {
      await self::genExecuteStandaloneProbes(
        $asset_type,
        $policy_enforcer_api,
        $caller,
        $context,
      );
    }

    return $result;
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

<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

module privacylib;

<<Oncalls('www_privacy_frameworks')>>
final class ThriftServiceMethodNameVirtualPolicyEnforcer
  implements IPolicyEnforcerAtThrift {
  const type TPrivacyLib = ThriftServiceMethodNamePrivacyLib;
  const PolicyEnforcerAssetClass ASSET_CLASS =
    PolicyEnforcerAssetClass::THRIFT_CLIENT;

  <<__Memoize(#MakeICInaccessible)>>
  private static function getPrivacyLibObject(
    ThriftServiceMethodNameAssetXID $asset_xid,
    string $sr_config_service_name,
  ): ThriftServiceMethodNamePrivacyLib {
    $privacy_lib = ThriftServiceMethodNamePrivacyLib::get(
      $asset_xid,
      $sr_config_service_name,
    );
    return $privacy_lib;
  }

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

  // TODO(duan): special usecase for this policyEnforcer
  private static function isTrusted(
    PolicyEnforcerCallerIdentity $caller,
    string $asset_type,
  )[zoned_shallow]: bool {
    return ThriftPolicyEnforcer::isTrusted($caller, $asset_type);
  }

  public static async function genExtractEnforcableAssets(
    string $func_name,
    ?IThriftStruct $args,
    string $smc_service_name,
  )[leak_safe]: Awaitable<keyset<string>> {
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
    $exception = null;

    list($result, $pl_failure) = await self::genPrivacyLibResults(
      $asset_type,
      $policy_enforcer_api,
      $caller,
      $context,
    );
    if ($pl_failure is nonnull && HH\legacy_is_truthy($pl_failure)) {
      $exception = $pl_failure->getException();
    }

    if ($exception is nonnull) {
      throw $exception;
    }

    // We only run probes if no exception is thrown
    // Pending Probes PrivacyLib Integration after PAAL Consolidation
    await self::genExecuteStandaloneProbes(
      $asset_type,
      $policy_enforcer_api,
      $caller,
      $context,
    );

    return $result;
  }

  <<StringMetadataExtractor>>
  private static async function genPrivacyLibResults(
    string $asset_type,
    string $policy_enforcer_api,
    PolicyEnforcerCallerIdentity $caller,
    PolicyEnforcerContext $context,
  )[zoned_local]: Awaitable<(TPolicyEnforcerResult, ?PrivacyLibModuleFailure)> {
    HH\set_frame_metadata('PAALC_NEW:THRIFT');
    // Thrift endpoint has this context set
    $asset_xid = $context->get(#ThriftPolicyZones)
      ?->getXID() ??
      // DeletionFrameworkNodeServiceAsyncHandler do not have context set,
      // but $api is the method name
      ThriftServiceMethodNameAssetXID::unsafeGet(
        $asset_type,
        $policy_enforcer_api,
      );

    /**
     * Incorrect sr_config_service_name will ONLY affect service with granular
     * annotation. We are not expecting null $sr_config_service_name here.
     * As a short-term solution, it will be default to ''. This ensure it
     * has minimum blast radius regarding infra change.
     */
    $sr_config_service_name =
      $context->get(#ThriftPolicyZones)?->getSmcServiceName() ?? '';

    self::customLogging($asset_xid, $caller);

    $privacy_lib =
      self::getPrivacyLibObject($asset_xid, $sr_config_service_name);
    $privacylib_failure = await $privacy_lib->genClientRPCWithTAE(
      null,
      $context,
      $caller,
      $policy_enforcer_api,
    );

    return tuple(
      shape(
        'allow' => $privacylib_failure is null,
        'is_privacy_safe' => self::isTrusted($caller, $asset_type),
        // Currently, Thrift is integrated with PrivacyLib for Zone2, which is not enforced yet globally.
        'ran_enforcing_policy_zone_check' => false,
        'logging_sampling_rate' => 0, // does not matter
      ),
      $privacylib_failure,
    );
  }

  private static function customLogging(
    ThriftServiceMethodNameAssetXID $xid,
    PolicyEnforcerCallerIdentity $caller,
  ): void {
    $service_name = $xid->getServiceName();
    if (
      coinflip(1000000) &&
      $service_name === 'InstagramGraphQLService' &&
      !PrivacyLibKS::isKilled(PLKS::THRIFT_GRAPHQL_CUSTOM_AUDIT)
    ) {
      if ($caller->getRawCallerBitmask() !== 1) {
        signal_log_in_psp_once(
          SignalDynamicLoggerDataset::PRIVACY_INFRASTRUCTURE,
          SignalDynamicLoggerProject::PRIVACYLIB_GRAPHQL_THRIFT_CUSTOM_AUDIT,
          'privacylib_graphql_thrift_custom_audit: '.
          (string)($caller->getRawCallerBitmask()),
          StackTrace::getCurrent(
            DEBUG_BACKTRACE_IGNORE_ARGS | DEBUG_BACKTRACE_PROVIDE_METADATA,
          )->__toString(),
        );
      }
    }
  }

  public static async function genProcessResponse(
    string $asset_type,
    string $policy_enforcer_api,
    PolicyEnforcerCallerIdentity $caller,
    PolicyEnforcerContext $context,
    mixed $response,
  ): Awaitable<void> {
    // Pending Probes PrivacyLib post-read Integration
    await self::genExecuteStandaloneProbes(
      $asset_type,
      $policy_enforcer_api,
      $caller,
      $context,
    );
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

final class ThriftPolicyZoneModuleContext
  implements IPolicyEnforcerModuleContext {
  public function __construct(
    private ThriftServiceMethodNameAssetXID $xid,
    private string $smcServiceName,
    private ?ICIPPContextForThriftFlows $explicitDynamicAnnotation_LEGACY =
      null,
  )[] {}

  public function getXID()[]: ThriftServiceMethodNameAssetXID {
    return $this->xid;
  }

  public function getSmcServiceName()[]: string {
    return $this->smcServiceName;
  }

  public function getServiceName()[]: string {
    return $this->xid->getServiceName();
  }

  public function getMethodName()[]: string {
    return $this->xid->getMethodName();
  }

  public function getExplicitDynamicZoneAnnotationSet_LEGACY(
  )[]: ?ICIPPContextForThriftFlows {
    return $this->explicitDynamicAnnotation_LEGACY;
  }
}

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

    if (!PrivacyLibKS::isKilled(PLKS::PAAL_CONSOLIDATION_THRIFT)) {
      // Run shadow traffic to validate results
      try {
        list($result, $pl_failure) = await self::genPrivacyLibResults(
          $asset_type,
          $policy_enforcer_api,
          $caller,
          $context,
        );

        $old_allow = $old_exception is null && $old_result['allow'];
        $new_allow = $result['allow'];

        $old_safe = $old_exception is null && $old_result['is_privacy_safe'];
        $new_safe = $result['is_privacy_safe'];

        if ($old_allow != $new_allow) {
          CategorizedOBC::typedBumpKey(
            ODSCategoryID::ODS_PRIVACYLIB,
            'www.privacylib.thrift.consolidation_mismatch.result',
          );
          PrivacyLibPaalConsolidationLogging::logResultMismatch(
            PaalConsolidationAssetClassName::THRIFT,
            $asset_type,
            $policy_enforcer_api,
            $caller,
            $old_allow,
            $new_allow,
            $pl_failure,
            $old_exception,
          );
        }

        if ($old_safe != $new_safe) {
          CategorizedOBC::typedBumpKey(
            ODSCategoryID::ODS_PRIVACYLIB,
            'www.privacylib.thrift.consolidation_mismatch.privacy_safe',
          );
          PrivacyLibPaalConsolidationLogging::logResultMismatch(
            PaalConsolidationAssetClassName::THRIFT_PRIVACY_SAFE,
            $asset_type,
            $policy_enforcer_api,
            $caller,
            $old_safe,
            $new_safe,
          );
        }
      } catch (Exception $e) {
        CategorizedOBC::typedBumpKey(
          ODSCategoryID::ODS_PRIVACYLIB,
          'www.privacylib.thrift.consolidation_exception',
        );
        PrivacyLibPaalConsolidationLogging::logEvaluationError(
          PaalConsolidationAssetClassName::THRIFT,
          $asset_type,
          $policy_enforcer_api,
          $caller,
          $e,
        );
      }
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

  <<StringMetadataExtractor>>
  private static async function genPolicyEnforcerResults(
    string $asset_type,
    string $policy_enforcer_api,
    PolicyEnforcerCallerIdentity $caller,
    PolicyEnforcerContext $context,
  )[zoned_local]: Awaitable<TPolicyEnforcerResult> {
    HH\set_frame_metadata('PAALC_OLD:THRIFT');
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

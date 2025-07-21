<?hh
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

final class ThriftPolicyEnforcer extends PolicyEnforcer {
  const PolicyEnforcerAssetClass ASSET_CLASS =
    PolicyEnforcerAssetClass::THRIFT_CLIENT;

  // NOTE: This PolicyEnforcer is under deprecation, please do not add new
  // modules. Please use corresponding PrivacyLib integration instead.

  const bool IS_THRIFT = true;
  const int ODS_ONE_IN = 100;
  const string CONFIG_READ_PATH = 'privacy/constraints/enforcer/thrift/';
  const keyset<string> THRIFT_SERVICES_TO_LOAD_CONFIG = keyset[
    'AdConvService',
    'AdalService',
    'BlobStore',
    'CFToolViewerAccessTokenService',
    'CachiusService',
    'DeletionFrameworkService',
    'LaserLeaf',
    'OmniPaymentEngineService',
    'PaymentEngineService',
    'RocksService',
    'SigmaService',
    'TitanMessagingService',
    'ZGatewayService',
    // for tests
    'FakeService',
  ];

  <<__Override>>
  public static function getNotConfigured(
    ?string $asset_type = null,
  )[leak_safe]: privacy_enforcer_ApiEnforcementConfig::TShape {
    // If the service is configured but the API is not, check whether
    // the service is in fail-open or fail-closed state
    $asset_type ??= '';
    $config = ThriftServiceDefaultApiEnforcementConfig::get();
    if ($config is null) {
      return
        ThriftServiceDefaultApiEnforcementConfig::getEmptyDefaultApiEnforcementConfig();
    }

    $default_config = $config['config'][$asset_type]['config']['www'] ?? null;
    if ($default_config is null) {
      return
        ThriftServiceDefaultApiEnforcementConfig::getEmptyDefaultApiEnforcementConfig();
    }

    $default_enforcement = coinflip($default_config['enforce_rate']);
    return shape(
      'enforce' => $default_enforcement,
      'allow_sampling_rate' => $default_config['allow_sampling_rate'],
      'deny_sampling_rate' => $default_config['deny_sampling_rate'],
      'whitelist' => keyset[],
      'isZeroQPSCandidate' => false,
    );
  }

  <<__Override>>
  public static function getTrustedCallersBitmask(
    string $asset_type, // This is actually the service name in this context
  )[zoned_shallow]: int {
    return
      PolicyEnforcerTrustedCallersConfig::get()['thrift']['www'][$asset_type] ??
      parent::getTrustedCallersBitmask($asset_type);
  }

  <<__Override>>
  protected static function getConditionallyAllowedCallersBitmask(
    string $asset_type, // This is actually the service name in this context
  )[leak_safe]: int {
    return
      PolicyEnforcerTrustedCallersConfig::get()['thrift']['wwwRequiresCIPPCheck'][$asset_type] ??
      0;
  }

  public static function getCallerName(
    string $asset_type, // This is actually the service name in this context
    ?int $caller,
  )[]: ?string {
    return ThriftServiceMethodNameVirtualPolicyEnforcer::getCallerName(
      $asset_type,
      $caller,
    );
  }

  <<__Override>>
  public static function getAssetTypeConfig(
    string $asset_type,
  )[leak_safe]: ?privacy_enforcer_AssetTypeConfig::TShape {
    if (!C\contains(self::THRIFT_SERVICES_TO_LOAD_CONFIG, $asset_type)) {
      return null;
    }
    return
      PolicyEnforcerAssetTypeConfig::get(self::CONFIG_READ_PATH.$asset_type);
  }

  <<__Override>>
  public static function getAssetTypeShadowConfig(
    string $asset_type,
    string $mutation_uuid,
  )[leak_safe]: ?privacy_enforcer_AssetTypeConfig::TShape {
    if (!C\contains(self::THRIFT_SERVICES_TO_LOAD_CONFIG, $asset_type)) {
      return null;
    }
    return PolicyEnforcerAssetTypeConfig::get(
      self::CONFIG_READ_PATH.$asset_type."___".$mutation_uuid,
    );
  }

}

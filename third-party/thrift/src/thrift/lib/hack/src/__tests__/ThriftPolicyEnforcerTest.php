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
module privacylib;

<<Oncalls('www_privacy_frameworks')>>
final class ThriftPolicyEnforcerTest extends WWWTest implements IOPEWWWTest {
  use ClassLevelTest;
  use TPolicyEnforcerTest;

  const int FAKE_CALLER_0 = 0;
  const int FAKE_CALLER_1 = 1;
  const int FAKE_CALLER_2 = 2;
  const int FAKE_CALLER_3 = 3;

  <<__Override>>
  public async function beforeEach(): Awaitable<void> {
    MockJustKnobs::setInt('privacy:emergency_sampling_backoff_www', 1);
    MockPZ2::disableSampling();
    MockPrivacyLib::neverDisableLibrary();

    // TODO(duan): pending consoldiation cleanup
    self::mockFunction(
      ThriftServiceMethodNameVirtualPolicyEnforcer::shouldKillConsolidation<>,
    )->mockReturn(true);
  }

  public function testTrustedCallersBitmask(): void {
    self::mockTrustedCallersConfig(shape(
      'wwwCommon' => 1,
      'fbcodeCommon' => 1,
    ));
    // Cannot find config for thrift, fallback
    expect(ThriftTrustedAPIEnforcerModule::getTrustedCallersBitmask('test'))
      ->toEqual(1);

    self::mockTrustedCallersConfig(shape(
      'wwwCommon' => 1,
      'fbcodeCommon' => 1,
      'thrift' => shape(
        'www' => dict[
          'test' => 100,
        ],
        'wwwRequiresCIPPCheck' => dict[],
        'fbcodeCommon' => 1,
      ),
    ));
    // Cannot find config for the requested service, fallback
    expect(ThriftTrustedAPIEnforcerModule::getTrustedCallersBitmask('foo'))
      ->toEqual(1);
    // Standard case
    expect(ThriftTrustedAPIEnforcerModule::getTrustedCallersBitmask('test'))
      ->toEqual(100);
  }

  public async function testNotConfiguredService(): Awaitable<void> {
    self::mockFunction(PolicyEnforcerAssetTypeConfig::get<>)
      ->mockReturn(null);

    self::mockFunction(ThriftServiceDefaultApiEnforcementConfig::get<>)
      ->mockReturn(null);

    self::mockFunctionStaticUNSAFE('ThriftPolicyEnforcer::genWeight')
      ->mockYield(1000);

    // Unconfigured service uses the global default
    expect(await ThriftPolicyEnforcer::genEnforcePolicyEnforcerAndPolicyZone(
      'FakeService',
      'fake_api',
      PolicyEnforcerCallerIdentity::unauthorizedCaller(),
      PolicyEnforcerContext::empty(),
    ))->toBePHPEqual(shape(
      'allow' => true,
      'is_privacy_safe' => false,
      'ran_enforcing_policy_zone_check' => false,
      'logging_sampling_rate' => 1000,
    ));
  }

  public async function testConfiguredServiceFailOpen(): Awaitable<void> {
    self::mockFunctionStaticUNSAFE('ThriftPolicyEnforcer::genWeight')
      ->mockYield(1000);
    self::mockFunction(PolicyEnforcerAssetTypeConfig::get<>)
      ->mockReturn(shape(
        'config' => dict[
          'a_different_fake_api' => shape(
            'enforce' => true,
            'allow_sampling_rate' => 0,
            'deny_sampling_rate' => 1,
            'whitelist' => keyset[],
            'denylist' => null,
            'isZeroQPSCandidate' => false,
            'bucket_sampling_rate' => dict[],
          ),
        ],
      ));

    expect(
      async () ==>
        await ThriftPolicyEnforcer::genEnforcePolicyEnforcerAndPolicyZone(
          'FakeService',
          'a_different_fake_api',
          PolicyEnforcerCallerIdentity::unauthorizedCaller(),
          PolicyEnforcerContext::empty(),
        ),
    )->toThrow(Exception::class);
    // Unconfigured API uses the global default
    expect(await ThriftPolicyEnforcer::genEnforcePolicyEnforcerAndPolicyZone(
      'FakeService',
      'another_fake_api',
      PolicyEnforcerCallerIdentity::unauthorizedCaller(),
      PolicyEnforcerContext::empty(),
    ))->toBePHPEqual(shape(
      'allow' => true,
      'is_privacy_safe' => false,
      'ran_enforcing_policy_zone_check' => false,
      'logging_sampling_rate' => 1000,
    ));
  }

  public async function testConfiguredServiceFailClosed(): Awaitable<void> {
    self::mockFunction(PolicyEnforcerAssetTypeConfig::get<>)
      ->mockReturn(shape(
        'config' => dict[
          'fake_api' => shape(
            'enforce' => true,
            'allow_sampling_rate' => 0,
            'deny_sampling_rate' => 1,
            'whitelist' => keyset[],
            'denylist' => null,
            'isZeroQPSCandidate' => false,
          ),
        ],
      ));

    self::mockFunction(ThriftServiceDefaultApiEnforcementConfig::get<>)
      ->mockReturn(
        HH\FIXME\UNSAFE_CAST<
          shape(
            'config' => shape(
              'AnotherFakeService' => shape(
                'config' => shape(
                  'www' => shape(
                    'allow_sampling_rate' => int,
                    'deny_sampling_rate' => int,
                    'enforce_rate' => int,
                  ),
                ),
              ),
            ),
          ),
          nothing,
        >(
          shape(
            'config' => shape(
              'AnotherFakeService' => shape(
                'config' => shape(
                  'www' => shape(
                    'enforce_rate' => 1,
                    'allow_sampling_rate' => 0,
                    'deny_sampling_rate' => 1,
                  ),
                ),
              ),
            ),
          ),
        ),
      );

    // Unconfigured API is enforced because the service is fail-closed
    expect(
      async () ==>
        await ThriftPolicyEnforcer::genEnforcePolicyEnforcerAndPolicyZone(
          'AnotherFakeService',
          'another_fake_api',
          PolicyEnforcerCallerIdentity::unauthorizedCaller(),
          PolicyEnforcerContext::empty(),
        ),
    )->toThrow(Exception::class);
  }

  public async function testEnforced(): Awaitable<void> {
    self::mockFunction(PolicyEnforcerAssetTypeConfig::get<>)
      ->mockReturn(HH\FIXME\UNSAFE_CAST<
        shape(
          'config' => dict<string, shape(
            'allow_sampling_rate' => int,
            'deny_sampling_rate' => int,
            'denylist' => null,
            'enforce' => bool,
            'isZeroQPSCandidate' => bool,
            'whitelist' => vec<nothing>,
          )>,
        ),
        nothing,
      >(shape(
        'config' => dict[
          'fake_api' => shape(
            'enforce' => true,
            'allow_sampling_rate' => 0,
            'deny_sampling_rate' => 1,
            'whitelist' => vec[],
            'denylist' => null,
            'isZeroQPSCandidate' => false,
          ),
        ],
      )));

    self::mockTrustedCallersConfig(shape(
      'wwwCommon' => self::getFakeBitmask(vec[self::FAKE_CALLER_0]),
      'fbcodeCommon' => 1,
      'thrift' => shape(
        'www' => dict[
          'FakeService' => self::getFakeBitmask(vec[self::FAKE_CALLER_1]),
        ],
        'wwwRequiresCIPPCheck' => dict[
          'FakeService' => self::getFakeBitmask(vec[self::FAKE_CALLER_2]),
        ],
        'fbcodeCommon' => 1,
      ),
    ));

    // Standard trusted caller
    expect(await ThriftPolicyEnforcer::genEnforcePolicyEnforcerAndPolicyZone(
      'FakeService',
      'fake_api',
      new PolicyEnforcerCallerIdentity(self::FAKE_CALLER_1),
      PolicyEnforcerContext::empty(),
    ))->toBePHPEqual(shape(
      'allow' => true,
      'is_privacy_safe' => true,
      'ran_enforcing_policy_zone_check' => false,
      'logging_sampling_rate' => 1,
    ));
    // Conditionally allowed caller even though privacy unsafe
    expect(await ThriftPolicyEnforcer::genEnforcePolicyEnforcerAndPolicyZone(
      'FakeService',
      'fake_api',
      new PolicyEnforcerCallerIdentity(self::FAKE_CALLER_2),
      PolicyEnforcerContext::empty(),
    ))->toBePHPEqual(shape(
      'allow' => true,
      'is_privacy_safe' => false,
      'ran_enforcing_policy_zone_check' => false,
      'logging_sampling_rate' => 1,
    ));
    // Other callers are denied because enforce is true
    expect(
      async () ==>
        await ThriftPolicyEnforcer::genEnforcePolicyEnforcerAndPolicyZone(
          'FakeService',
          'fake_api',
          new PolicyEnforcerCallerIdentity(self::FAKE_CALLER_3),
          PolicyEnforcerContext::empty(),
        ),
    )->toThrow(Exception::class);
  }

  public async function testGetCallerName(): Awaitable<void> {
    expect(ThriftPolicyEnforcer::getCallerName('FakeService', null))
      ->toBeNull();
    expect(ThriftPolicyEnforcer::getCallerName(
      'FakeService',
      LaserCaller::ENT_LOADER,
    ))
      ->toBeNull();
    expect(ThriftPolicyEnforcer::getCallerName('CachiusService', null))
      ->toBeNull();
    expect(ThriftPolicyEnforcer::getCallerName(
      'CachiusService',
      CachiusCaller::ENT_LOADER,
    ))
      ->toEqual('ENT_LOADER');
    expect(ThriftPolicyEnforcer::getCallerName('CachiusService', 600))
      ->toBeNull();

    expect(
      ThriftPolicyEnforcer::getCallerName(
        'AdalService',
        AdalMySQLThriftCaller::RAW_SQL_CALLER,
      ),
    )->toEqual('RAW_SQL_CALLER');
  }

  public async function testThriftMethodXID(): Awaitable<void> {
    $mock = self::mockFunction(ThriftServiceMethodNamePrivacyLib::get<>);

    await ThriftPolicyEnforcer::genEnforcePolicyEnforcerAndPolicyZone(
      'FakeService',
      'fake_api',
      new PolicyEnforcerCallerIdentity(null),
      PolicyEnforcerContext::builder()
        ->add(
          #ThriftPolicyZones,
          new ThriftPolicyZoneModuleContext(
            ThriftServiceMethodNameAssetXID::unsafeGet(
              'FakeService',
              'fake_method',
            ),
            'FakeService',
          ),
        )
        ->build(),
    );
    expect($mock)->toBeCalledWithInAnyOrder(
      vec[vec[
        ThriftServiceMethodNameAssetXID::get('FakeService', 'fake_method'),
        'FakeService',
      ]],
    );
  }

  public async function testThriftClientMethodNameNotFiltered(
  ): Awaitable<void> {
    $mock = self::mockFunction(ThriftServiceMethodNamePrivacyLib::get<>)
      ->assertCanPassthroughAndBlockMocks();
    self::mockClassStaticMethodUNSAFE(
      PolicyEnforcer::class,
      'shouldExecuteModule',
    )
      ->mockImplementation(($_, $identifier) ==> {
        return $identifier !== 'thrift_tae';
      });
    $sr_client = await SRClient(
      "client_1",
      PrivacyLibEcho\PrivacyLibEchoServiceAsyncClient::factory(),
      "sr_config_1",
    )->gen();
    await (
      new TClientPoliciedAsyncHandler(
        'sr_config_1',
        nameof PrivacyLibEcho\PrivacyLibEchoServiceAsyncClient,
        $sr_client,
        nameof self,
      )
    )->genBefore('PrivacyLibEchoService', 'echo');

    expect($mock)->toBeCalledWithInAnyOrder(
      vec[
        vec[
          ThriftServiceMethodNameAssetXID::get('PrivacyLibEchoService', 'echo'),
          'sr_config_1',
        ],
        vec[
          ThriftServiceMethodNameAssetXID::get('PrivacyLibEchoService', 'echo'),
          'sr_config_1',
        ],
      ],
    );
  }

  public async function testPZ2MetadataLogging(): Awaitable<void> {
    $mock = self::mockFunction(PZ2FlowsToLoggingXSU::genLog<>);
    self::mockFunction(PrivacyLibArtifactLoader::getArtifactForXID<>)
      ->mockReturn(
        PLXID_asset___thrift_service_name_CAAIGLoginServicePrivacyLibArtifact::class,
      );
    $sr_client = await SRClient(
      "client_1",
      PrivacyLibEcho\PrivacyLibEchoServiceAsyncClient::factory(),
      "foo/bar",
    )->gen();
    $handler = new TClientPoliciedAsyncHandler(
      'foo/bar',
      nameof PrivacyLibEcho\PrivacyLibEchoServiceAsyncClient,
      $sr_client,
      nameof self,
    );
    await ThriftPolicyEnforcer::genEnforcePolicyEnforcerAndPolicyZone(
      'FakeService',
      'fake_api',
      new PolicyEnforcerCallerIdentity(null),
      PolicyEnforcerContext::builder()
        ->add(
          #ThriftPolicyZones,
          new ThriftPolicyZoneModuleContext(
            ThriftServiceMethodNameAssetXID::unsafeGet(
              'FakeService',
              'fake_method',
            ),
            'FakeService',
          ),
        )
        ->add(
          #PZ2,
          PZ2ModuleContext::get()->metadataLogging(
            TClientAsyncHandlerFlowsTo::getMetadataThunk($handler),
          ),
        )
        ->build(),
    );
    await PSP()->genRunFromUnitTest(); // flush logs
    expect($mock)->wasCalledWithArgumentsPassingNTimes(
      ($_event, $_sample_rate, $specific_meta) ==> {
        return $specific_meta['thrift_caller_service_id'] === 'foo/bar';
      },
      2,
    );
  }

  public async function testPZ2MetadataLoggingThroughHandler(
  ): Awaitable<void> {
    $mock = self::mockFunction(PZ2FlowsToLoggingXSU::genLog<>);
    self::mockFunction(PrivacyLibArtifactLoader::getArtifactForXID<>)
      ->assertCanPassthroughAndBlockMocks();
    $sr_client = await SRClient(
      "client_1",
      PrivacyLibEcho\PrivacyLibEchoServiceAsyncClient::factory(),
      "sr_config_1",
    )->gen();
    await (
      new TClientPoliciedAsyncHandler(
        'sr_config_1',
        nameof PrivacyLibEcho\PrivacyLibEchoServiceAsyncClient,
        $sr_client,
        nameof self,
      )
    )->genBefore('FakeService', 'fake_method');

    await PSP()->genRunFromUnitTest(); // flush logs
    expect($mock)->wasCalledWithArgumentsPassingNTimes(
      ($_event, $_sample_rate, $specific_meta) ==> {
        return $specific_meta['thrift_caller_service_id'] === 'sr_config_1';
      },
      2,
    );
  }

  public async function testClientIDLogging(): Awaitable<void> {
    $mock = self::mockFunction(PZ2UniverseFunctionalLogger::genLog<>);
    self::mockFunction(PrivacyLibArtifactLoader::getArtifactForXID<>)
      ->assertCanPassthroughAndBlockMocks();
    $client_id = 'client_1';

    $sr_client = await SRClient(
      $client_id,
      PrivacyLibEcho\PrivacyLibEchoServiceAsyncClient::factory(),
      "sr_config_1",
    )->gen();
    HTTPHeaders::overrideForTest(
      dict[HTTPRequestHeader::CLIENT_ID => $client_id],
    );
    await (
      new TClientPoliciedAsyncHandler(
        'sr_config_1',
        nameof PrivacyLibEcho\PrivacyLibEchoServiceAsyncClient,
        $sr_client,
        nameof self,
      )
    )->genBefore('FakeService', 'fake_method');

    await PSP()->genRunFromUnitTest(); // flush logs
    expect($mock)->wasCalledWithArgumentsPassingNTimes(
      ($log, $_sample_rate) ==> {
        return Shapes::idx($log, 'client_id') === $client_id;
      },
      2,
    );
  }
}

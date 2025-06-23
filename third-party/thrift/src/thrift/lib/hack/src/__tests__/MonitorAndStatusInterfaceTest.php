<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift_hack')>>
final class MonitorAndStatusInterfaceTest extends WWWTest {

  const type TTestReturnValues = shape(
    ?'statusDetails' => string,
    ?'counters' => dict<string, int>,
    ?'load' => float,
  );

  private static string $fb303StatusDetails =
    'mocked_status_details_from_fb303';
  private static dict<string, int> $fb303Counter =
    dict['mocked_counter_fb303' => 1];
  private static float $fb303Load = 1.0;

  private static string $defaultStatusDetails =
    'mocked_status_details_from_DefaultStatus';
  private static dict<string, int> $defaultMonitorCounter =
    dict['mocked_counter_DefaultMonitor' => 1];
  private static float $defaultStatusLoad = 2.0;

  private static string $customStatusDetails =
    'mocked_status_details_from_custom_status';
  private static dict<string, int> $customMonitorCounter =
    dict['mocked_status_details_from_custom_monitor' => 1];
  private static float $customStatusLoad = 5.0;

  <<__LateInit>> private Facebook\Thrift\StatusAsyncClient $statusClient;
  <<__LateInit>> private Facebook\Thrift\MonitorAsyncClient $monitorClient;
  <<__LateInit>>
  private Facebook\Thrift\StatusAsyncClient $statusClientWithOverride;
  <<__LateInit>>
  private Facebook\Thrift\MonitorAsyncClient $monitorClientWithOverride;

  <<__Override>>
  public static async function createData(): Awaitable<void> {
    self::mockFunction(
      meth_caller(DeprecatedFacebookAsyncBase::class, 'getStatusDetails'),
    )->mockYield(self::$fb303StatusDetails);
    self::mockFunction(
      meth_caller(DeprecatedFacebookAsyncBase::class, 'getCounters'),
    )
      ->mockYield(self::$fb303Counter);
    self::mockFunction(
      meth_caller(DeprecatedFacebookAsyncBase::class, 'getLoad'),
    )
      ->mockYield(self::$fb303Load);

    self::mockFunction(meth_caller(DefaultStatus::class, 'getStatusDetails'))
      ->mockYield(self::$defaultStatusDetails);
    self::mockFunction(meth_caller(DefaultStatus::class, 'getLoad'))
      ->mockYield(self::$defaultStatusLoad);
    self::mockFunction(meth_caller(DefaultMonitor::class, 'getCounters'))
      ->mockYield(self::$defaultMonitorCounter);

    $mocked_status = mock(DefaultStatus::class)
      ->mockYield('getStatusDetails', self::$customStatusDetails)
      ->mockYield('getLoad', self::$customStatusLoad);
    $mocked_monitor = mock(DefaultMonitor::class)
      ->mockYield('getCounters', self::$customMonitorCounter);

    self::mockFunctionStaticUNSAFE(
      "MonitorAndStatusInterfaceTestServerWithOverrides::getStatusInterface",
    )->mockReturn($mocked_status);
    self::mockFunctionStaticUNSAFE(
      "MonitorAndStatusInterfaceTestServerWithOverrides::getMonitorInterface",
    )->mockReturn($mocked_monitor);
  }

  <<__Override>>
  public async function beforeEach(): Awaitable<void> {
    list($this->statusClient, $_) = LocalThriftConnection::setup<
      Facebook\Thrift\StatusAsyncClient,
      MonitorAndStatusInterfaceTestServer,
    >();
    list($this->monitorClient, $_) = LocalThriftConnection::setup<
      Facebook\Thrift\MonitorAsyncClient,
      MonitorAndStatusInterfaceTestServer,
    >();
    list($this->statusClientWithOverride, $_) = LocalThriftConnection::setup<
      Facebook\Thrift\StatusAsyncClient,
      MonitorAndStatusInterfaceTestServerWithOverrides,
    >();
    list($this->monitorClientWithOverride, $_) = LocalThriftConnection::setup<
      Facebook\Thrift\MonitorAsyncClient,
      MonitorAndStatusInterfaceTestServerWithOverrides,
    >();
  }

  public static function provideTestProcessors(): dict<string, shape(
    'processor' => ThriftAsyncProcessor,
    'result' => self::TTestReturnValues,
    'result_with_infra_rpc' => self::TTestReturnValues,
    'result_with_overrides' => self::TTestReturnValues,
  )> {
    $old_return_values = shape(
      'statusDetails' => self::$fb303StatusDetails,
      'counters' => self::$fb303Counter,
      'load' => self::$fb303Load,
    );
    $new_return_values = shape(
      'statusDetails' => self::$defaultStatusDetails,
      'counters' => self::$defaultMonitorCounter,
      'load' => self::$defaultStatusLoad,
    );
    $use_custom_load = (self::TTestReturnValues $result) ==> {
      $result['load'] = 3.0;
      return $result;
    };
    $custom_return_values = shape(
      'statusDetails' => self::$customStatusDetails,
      'counters' => self::$customMonitorCounter,
      'load' => self::$customStatusLoad,
    );

    return dict[
      'Inherits Fb303 in IDL and Handler' => shape(
        'processor' => new TestExtendsFacebookServiceAsyncProcessor(
          new TestExtendsFacebookServiceHandler("test"),
        ),
        'result' => $old_return_values,
        /**
         * Since the handler extends FacebookService, it will continue to use
         * it until the IDL is updated to remove inheritance from FacebookService
         */
        'result_with_infra_rpc' => $old_return_values,
        'result_with_overrides' => $old_return_values,
      ),
      'Inherits Fb303 in IDL and Handler with overrides' => shape(
        'processor' => new TestExtendsFacebookServiceAsyncProcessor(
          new TestExtendsFacebookServiceHandlerWithOverrides("test"),
        ),
        'result' => $use_custom_load($old_return_values),
        /**
         * Since the handler extends FacebookService, it will continue to use
         * it until the IDL is updated to remove inheritance from FacebookService
         */
        'result_with_infra_rpc' => $use_custom_load($old_return_values),
        'result_with_overrides' => $use_custom_load($old_return_values),
      ),
      'Inherits Fb303 only in Handler' => shape(
        'processor' => new TestExtendsFacebookServiceInHandlerAsyncProcessor(
          new TestExtendsFacebookServiceInHandlerImpl("test"),
        ),
        /**
         * Even though the handler extends FacebookService, it will not be used
         * as the processor doesn't have the process_<fname> methods for the fb303 methods.
         * It will fail with exception.
         */
        'result' => shape(),
        /**
         * After the infra RPCs are injected, DefaultStatus and DefaultMonitor will be used.
         */
        'result_with_infra_rpc' => $new_return_values,
        // Uses the custom implementation
        'result_with_overrides' => $custom_return_values,
      ),
      'Inherits Fb303 only in Handler with overrides' => shape(
        'processor' => new TestExtendsFacebookServiceInHandlerAsyncProcessor(
          new TestExtendsFacebookServiceInHandlerImplWithOverrides("test"),
        ),
        'result' => shape(),
        /**
         * After the infra RPCs are injected, DefaultStatus and DefaultMonitor will be used.
         * Override will be ignored.
         *
         */
        'result_with_infra_rpc' => $new_return_values,
        // Uses the custom implementation
        'result_with_overrides' => $custom_return_values,
      ),
      'Doesn\'t Inherit from Fb303 ' => shape(
        'processor' =>
          new TestDummyServiceAsyncProcessor(new TestDummyServiceHandler()),
        'result' => shape(),
        'result_with_infra_rpc' => $new_return_values,
        // Uses the custom implementation
        'result_with_overrides' => $custom_return_values,
      ),
      'Has similar methods like infra RPCs ' => shape(
        'processor' => new TestDummyServiceWithReservedMethodNameAsyncProcessor(
          new TestDummyServiceWithReservedMethodNameHandler(),
        ),
        /**
         * A user defined function in the service could have name
         * that is reserved for infra RPCs despite the
         * lint here : https://fburl.com/code/8lpre7tq
         *
         * In that case, user provided implementation gets precedence.
         * Since the AsyncProcessor will have process_<fname> for this defined method
         * with reserved name, the rpc is successfully even without injected RPCs.
         */
        'result' => $use_custom_load(shape()),
        'result_with_infra_rpc' => $use_custom_load($new_return_values),
        // Uses the custom implementation
        'result_with_overrides' => $use_custom_load($custom_return_values),
      ),
    ];
  }

  private async function genExecRequests(
    ThriftAsyncProcessor $processor,
    self::TTestReturnValues $result,
    bool $use_server_with_overrides,
  ): Awaitable<void> {
    // Reset the event handler after every request
    // to avoid any side effects from previous request
    $event_handler = new TProcessorEventHandler();
    $processor->setEventHandler($event_handler);
    if ($use_server_with_overrides) {
      $status_client = $this->statusClientWithOverride;
      $monitor_client = $this->monitorClientWithOverride;
    } else {
      $status_client = $this->statusClient;
      $monitor_client = $this->monitorClient;
    }
    if (Shapes::keyExists($result, 'statusDetails')) {
      expect(await $status_client->getStatusDetails())->toEqual(
        $result['statusDetails'],
      );
    } else {
      expect(async () ==> await $status_client->getStatusDetails())
        ->toThrow(
          TApplicationException::class,
          'Function getStatusDetails is not found in the service',
        );
    }
    $processor->setEventHandler($event_handler);
    if (Shapes::keyExists($result, 'load')) {
      expect(await $status_client->getLoad())->toEqual($result['load']);
    } else {
      expect(async () ==> await $status_client->getLoad())
        ->toThrow(
          TApplicationException::class,
          'Function getLoad is not found in the service',
        );
    }
    $processor->setEventHandler($event_handler);
    if (Shapes::keyExists($result, 'counters')) {
      expect(await $monitor_client->getCounters())->toEqual(
        $result['counters'],
      );
    } else {
      expect(async () ==> await $monitor_client->getCounters())
        ->toThrow(
          TApplicationException::class,
          'Function getCounters is not found in the service',
        );
    }
  }

  <<DataProvider('provideTestProcessors')>>
  public async function testInfraRPCs(
    ThriftAsyncProcessor $processor,
    self::TTestReturnValues $result,
    self::TTestReturnValues $result_with_infra_rpc,
    self::TTestReturnValues $result_with_overrides,
  ): Awaitable<void> {
    self::mockFunction(
      meth_caller(MonitorAndStatusInterfaceTestServer::class, 'getProcessor'),
    )->mockReturn($processor);

    MockJustKnobs::setBool(
      'thrift/hack:fb303_FacebookService_deprecation',
      false,
    );
    await $this->genExecRequests($processor, $result, false);
    await $this->genExecRequests($processor, $result, true);

    MockJustKnobs::setBool(
      'thrift/hack:fb303_FacebookService_deprecation',
      true,
    );
    await $this->genExecRequests($processor, $result_with_infra_rpc, false);
    await $this->genExecRequests($processor, $result_with_overrides, true);
  }
}

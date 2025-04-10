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

  <<__LateInit>> private Facebook\Thrift\StatusAsyncClient $statusClient;
  <<__LateInit>> private Facebook\Thrift\MonitorAsyncClient $monitorClient;

  <<__Override>>
  public static async function createData(): Awaitable<void> {
    self::mockFunction(
      meth_caller(FacebookAsyncBase::class, 'getStatusDetails'),
    )->mockYield(self::$fb303StatusDetails);
    self::mockFunction(meth_caller(FacebookAsyncBase::class, 'getCounters'))
      ->mockYield(self::$fb303Counter);
    self::mockFunction(meth_caller(FacebookAsyncBase::class, 'getLoad'))
      ->mockYield(self::$fb303Load);
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
  }

  public static function provideTestProcessors(): dict<string, shape(
    'processor' => ThriftAsyncProcessor,
    'result' => self::TTestReturnValues,
  )> {
    $old_return_values = shape(
      'statusDetails' => self::$fb303StatusDetails,
      'counters' => self::$fb303Counter,
      'load' => self::$fb303Load,
    );
    $use_custom_load = (self::TTestReturnValues $result) ==> {
      $result['load'] = 3.0;
      return $result;
    };

    return dict[
      'Inherits Fb303 in IDL and Handler' => shape(
        'processor' => new TestExtendsFacebookServiceAsyncProcessor(
          new TestExtendsFacebookServiceHandler("test"),
        ),
        'result' => $old_return_values,
      ),
      'Inherits Fb303 in IDL and Handler with overrides' => shape(
        'processor' => new TestExtendsFacebookServiceAsyncProcessor(
          new TestExtendsFacebookServiceHandlerWithOverrides("test"),
        ),
        'result' => $use_custom_load($old_return_values),
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
      ),
      'Inherits Fb303 only in Handler with overrides' => shape(
        'processor' => new TestExtendsFacebookServiceInHandlerAsyncProcessor(
          new TestExtendsFacebookServiceInHandlerImplWithOverrides("test"),
        ),
        'result' => shape(),
      ),
      'Doesn\'t Inherit from Fb303 ' => shape(
        'processor' =>
          new TestDummyServiceAsyncProcessor(new TestDummyServiceHandler()),
        'result' => shape(),
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
      ),
    ];
  }

  <<DataProvider('provideTestProcessors')>>
  public async function testGetStatusDetailsRPC(
    ThriftAsyncProcessor $processor,
    self::TTestReturnValues $result,
  ): Awaitable<void> {
    self::mockFunction(
      meth_caller(MonitorAndStatusInterfaceTestServer::class, 'getProcessor'),
    )->mockReturn($processor);
    if (Shapes::keyExists($result, 'statusDetails')) {
      expect(await $this->statusClient->getStatusDetails())->toEqual(
        $result['statusDetails'],
      );
    } else {
      expect(async () ==> await $this->statusClient->getStatusDetails())
        ->toThrow(
          TApplicationException::class,
          'Function getStatusDetails is not found in the service',
        );
    }
  }

  <<DataProvider('provideTestProcessors')>>
  public async function testGetLoadRPC(
    ThriftAsyncProcessor $processor,
    self::TTestReturnValues $result,
  ): Awaitable<void> {
    self::mockFunction(
      meth_caller(MonitorAndStatusInterfaceTestServer::class, 'getProcessor'),
    )->mockReturn($processor);
    if (Shapes::keyExists($result, 'load')) {
      expect(await $this->statusClient->getLoad())->toEqual($result['load']);
    } else {
      expect(async () ==> await $this->statusClient->getLoad())
        ->toThrow(
          TApplicationException::class,
          'Function getLoad is not found in the service',
        );
    }
  }

  <<DataProvider('provideTestProcessors')>>
  public async function testGetCountersRPC(
    ThriftAsyncProcessor $processor,
    self::TTestReturnValues $result,
  ): Awaitable<void> {
    self::mockFunction(
      meth_caller(MonitorAndStatusInterfaceTestServer::class, 'getProcessor'),
    )->mockReturn($processor);
    if (Shapes::keyExists($result, 'counters')) {
      expect(await $this->monitorClient->getCounters())->toEqual(
        $result['counters'],
      );
    } else {
      expect(async () ==> await $this->monitorClient->getCounters())
        ->toThrow(
          TApplicationException::class,
          'Function getCounters is not found in the service',
        );
    }
  }

}

<?hh

// @lint-ignore-every GEN_PREFIX method name need to match the location_transparency thrift method names
<<Oncalls('thrift_hack')>>
final class MonitorAndStatusInterfaceTestServer extends METAThriftServer {
  const type TProcessor = ThriftAsyncProcessor;
  const string SERVICE_ID = 'MonitorAndStatusInterfaceServer';
  // Mock this method to customize the processor
  <<__Override>>
  public function getProcessor(): ThriftAsyncProcessor {
    throw new Exception('Not implemented');
  }

  <<__Override>>
  public static function getService(): string {
    return self::SERVICE_ID;
  }
}

// TEST CASE : Simple service with no infra RPCs
<<Oncalls('thrift_hack')>>
final class TestDummyServiceHandler implements TestDummyServiceAsyncIf {}

// TEST CASE : Simple service with user RPCs similar to infra RPCs
final class TestDummyServiceWithReservedMethodNameHandler
  implements TestDummyServiceWithReservedMethodNameAsyncIf {
  public async function getLoad(): Awaitable<float> {
    return 3.0;
  }
}
// TEST CASE : Service which inherits from FacebookService
<<Oncalls('thrift_hack')>>
final class TestExtendsFacebookServiceHandler
  extends DeprecatedFacebookAsyncBase
  implements TestExtendsFacebookServiceAsyncIf {}

// TEST CASE : Service which inherits from FacebookService and overrides some methods
// This could be a case where service doesn't use the default implementation
// and provides its own implementation for all the methods
<<Oncalls('thrift_hack')>>
final class TestExtendsFacebookServiceHandlerWithOverrides
  extends DeprecatedFacebookAsyncBase
  implements TestExtendsFacebookServiceAsyncIf {

  <<__Override>>
  public async function getLoad(): Awaitable<float> {
    return 3.0;
  }
}

// TEST CASE : Service which doesn't inherit from FacebookService in IDL
//              but extends FacebookAsyncBase in handler.
<<Oncalls('thrift_hack')>>
final class TestExtendsFacebookServiceInHandlerImpl
  extends DeprecatedFacebookAsyncBase
  implements TestExtendsFacebookServiceInHandlerAsyncIf {}

// TEST CASE : Service which doesn't inherit from FacebookService in IDL
//              but extends FacebookAsyncBase in handler and has overrides.
<<Oncalls('thrift_hack')>>
final class TestExtendsFacebookServiceInHandlerImplWithOverrides
  extends DeprecatedFacebookAsyncBase
  implements TestExtendsFacebookServiceInHandlerAsyncIf {

  <<__Override>>
  public async function getLoad(): Awaitable<float> {
    throw new Exception('Not implemented');
  }
}

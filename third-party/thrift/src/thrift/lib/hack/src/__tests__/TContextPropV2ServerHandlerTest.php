<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('artillery')>>
final class TContextPropV2ServerHandlerTest extends WWWTest {

  <<__Override>>
  public async function beforeEach(): Awaitable<void> {
    enable_artillery_controller_observer();
  }

  /**
   * Test that handlerException calls observeException on the artillery
   * controller observer with the exception
   */
  public function testHandlerExceptionObservesException(): void {
    $mock_thrift_server = mock(ThriftServer::class);
    $handler = new TContextPropV2ServerHandler($mock_thrift_server, shape());

    $test_exception = new Exception('Test exception message');

    // Execute: Call handlerException
    $handler->handlerException(null, 'test_fn', $test_exception);

    // Assert: Verify exception was observed
    $observer = get_artillery_controller_observer();
    expect($observer->getFirstException())->toEqual($test_exception);
  }

  /**
   * Test that handlerError calls observeException on the artillery
   * controller observer with the exception
   */
  public function testHandlerErrorObservesException(): void {
    $mock_thrift_server = mock(ThriftServer::class);
    $handler = new TContextPropV2ServerHandler($mock_thrift_server, shape());

    $test_exception = new Exception('Test error message');

    // Execute: Call handlerError
    $handler->handlerError(null, 'test_fn', $test_exception);

    // Assert: Verify exception was observed
    $observer = get_artillery_controller_observer();
    expect($observer->getFirstException())->toEqual($test_exception);
  }

  /**
   * Test that handlerException passes through different exception types
   */
  public function testHandlerExceptionWithTApplicationException(): void {
    $mock_thrift_server = mock(ThriftServer::class);
    $handler = new TContextPropV2ServerHandler($mock_thrift_server, shape());

    $thrift_exception = new TApplicationException('Thrift application error');

    // Execute: Call handlerException with a TApplicationException
    $handler->handlerException(null, 'test_fn', $thrift_exception);

    // Assert: Verify TApplicationException was observed
    $observer = get_artillery_controller_observer();
    expect($observer->getFirstException())->toEqual($thrift_exception);
  }

  /**
   * Test that handlerError passes through different exception types
   */
  public function testHandlerErrorWithTApplicationException(): void {
    $mock_thrift_server = mock(ThriftServer::class);
    $handler = new TContextPropV2ServerHandler($mock_thrift_server, shape());

    $thrift_exception = new TApplicationException('Thrift application error');

    // Execute: Call handlerError with a TApplicationException
    $handler->handlerError(null, 'test_fn', $thrift_exception);

    // Assert: Verify TApplicationException was observed
    $observer = get_artillery_controller_observer();
    expect($observer->getFirstException())->toEqual($thrift_exception);
  }

  /**
   * Test that only the first exception is captured by the observer
   */
  public function testOnlyFirstExceptionIsCaptured(): void {
    $mock_thrift_server = mock(ThriftServer::class);
    $handler = new TContextPropV2ServerHandler($mock_thrift_server, shape());

    $exception1 = new Exception('First exception');
    $exception2 = new Exception('Second exception');
    $exception3 = new Exception('Third exception');

    // Execute: Call both methods multiple times
    $handler->handlerException(null, 'fn1', $exception1);
    $handler->handlerError(null, 'fn2', $exception2);
    $handler->handlerException(null, 'fn3', $exception3);

    // Assert: Only the first exception is stored
    $observer = get_artillery_controller_observer();
    expect($observer->getFirstException())->toEqual($exception1);
  }
}

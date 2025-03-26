<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

abstract final class ThriftClientTestUtils {

  public static function mockRPCResponse<T>(
    ?T $response,
    classname<ThriftClientBase> $cls = ThriftClientBase::class,
  ): void {
    WWWTest::mockFunctionStaticUNSAFE($cls.'::genAwaitResponse')
      ->mockYield(tuple($response, null));
  }

  public static function mockRPCWithException(
    Exception $ex,
    bool $after_execution = false,
    classname<ThriftClientBase> $cls = ThriftClientBase::class,
  ): void {

    $mock_func = WWWTest::mockFunctionStaticUNSAFE($cls.'::genAwaitResponse');
    if ($after_execution) {
      $mock_func->mockYieldThenThrow(() ==> $ex);
    } else {
      $mock_func->mockImplementation(
        () ==> {
          throw $ex;
        },
      );
    }
  }
}

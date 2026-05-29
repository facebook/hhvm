<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
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
 */

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

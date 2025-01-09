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

<<Oncalls('thrift')>>
final class TTransportExceptionTestCase extends WWWTest {

  use ClassLevelTest;

  public function testSimple(): void {
    $short_message = null;
    $code = null;
    try {
      throw new TTransportException(
        "This is a dummy message",
        TTransportException::UNKNOWN,
        "dummy",
      );
    } catch (TTransportException $ex) {
      $short_message = $ex->getShortMessage();
      $code = $ex->getCode();
    }

    expect($short_message)->toBePHPEqual('dummy');
    expect($code)->toBePHPEqual(TTransportException::UNKNOWN);
  }
}

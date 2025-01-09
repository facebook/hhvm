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
final class TMemoryBufferTest extends WWWTest {

  use ClassLevelTest;

  public static function providePeekCases(): dict<string, shape(
    'buffer_contents' => string,
    'length' => int,
    'expected' => string,
  )> {
    return dict[
      'a single character' => shape(
        'buffer_contents' => 'abc',
        'length' => 1,
        'expected' => 'a',
      ),
      'a multiple characters' => shape(
        'buffer_contents' => 'abc',
        'length' => 2,
        'expected' => 'ab',
      ),
      'an empty buffer' => shape(
        'buffer_contents' => '',
        'length' => 1,
        'expected' => '',
      ),
    ];
  }

  <<DataProvider('providePeekCases')>>
  public function testPeek(
    string $buffer_contents,
    int $length,
    string $expected,
  ): void {
    $buffer = new TMemoryBuffer($buffer_contents);
    expect($buffer->peek($length))->toBePHPEqual($expected);
  }
}

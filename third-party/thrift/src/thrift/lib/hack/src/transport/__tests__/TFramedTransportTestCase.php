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

//////////////////////////////////////
// Test TFramedTransport
//
// @owner asharma
/////////////////////////////////////

<<Oncalls('thrift')>>
final class TFramedTransportTestCase extends WWWTest {

  use ClassLevelTest;

  public static function providerTFramedTransport(
  ): vec<(classname<TTransport>, string)> {
    $transports = vec[TBufferedTransport::class, TFramedTransport::class];

    $message_array = vec[];
    /* Try all messages of sizes from 0 to 4k */
    for ($i = 1; $i <= 12; $i += 1) {
      /* Try odd message sizes */
      $size = (1 << $i) + 1;
      $message = '';
      for ($j = 0; $j < $size; $j += 1) {
        $message .= ($j % 10);
      }
      $message_array[] = $message;
    }

    $tests = vec[];
    foreach ($transports as $transport) {
      foreach ($message_array as $message) {
        $tests[] = tuple($transport, $message);
      }
    }
    return $tests;
  }

  <<DataProvider('providerTFramedTransport')>>
  public function testTFramedTransport(
    classname<TTransport> $tname,
    string $message,
  ): void {
    $buffer1 = new TMemoryBuffer($message);
    $buffer2 = new TMemoryBuffer();

    switch ($tname) {
      case TBufferedTransport::class:
        $transport = new TBufferedTransport<TMemoryBuffer>($buffer2);
        break;
      case TFramedTransport::class:
        $transport = new TFramedTransport($buffer2);
        break;
    }
    /* Write data in 16 byte frames */
    do {
      $transport->write($message);
      $transport->flush();
      $message = PHP\substr($message, 16);
    } while ($message);
    $transport->flush();

    $len = Str\length($buffer1->getBuffer());

    /* Verify that directly reading from the TMemoryBuffer and
     * $transport return identical data
     */
    $data1 = $buffer1->read($len);
    /* Read data in different chunk sizes */
    $data2 = '';
    $consumed = 0;
    for ($size = 1; $consumed < $len; $size <<= 1) {
      $size = Math\minva($size, $len - $consumed);
      $out = $transport->read($size);

      /* Make sure putBack() works */
      $transport->putBack($out);
      $out = $transport->read($size);

      $data2 .= $out;
      $consumed += Str\length($out);
    }
    expect($consumed)->toBePHPEqual(
      $len,
      '%s',
      $tname.' returns the same data of the same length',
    );
    expect($data2)->toBePHPEqual($data1, '%s', $tname.' returns the same data');
  }
}

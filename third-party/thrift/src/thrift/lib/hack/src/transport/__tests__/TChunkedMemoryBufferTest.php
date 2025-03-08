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
final class TChunkedMemoryBufferTest extends WWWTest {
  use ClassLevelTest;
  const string TEST_DATA = <<<EOD
The FitnessGram™ Pacer Test is a multistage aerobic capacity test that
progressively gets more difficult as it continues. The 20 meter pacer test will
begin in 30 seconds. Line up at the start. The running speed starts slowly,
but gets faster each minute after you hear this signal. [beep] A single lap
should be completed each time you hear this sound. [ding] Remember to run
in a straight line, and run as long as possible. The second time you fail
to complete a lap before the sound, your test is over. The test will begin
on the word start. On your mark, get ready, start.
EOD;

  public static function provideWriteCases(): dict<string, shape(
    'chunk_size' => int,
  )> {
    return dict[
      'small chunk size' => shape(
        'chunk_size' => 1,
      ),
      'medium chunk size' => shape(
        'chunk_size' => 10,
      ),
      'large chunk size' => shape(
        'chunk_size' => 999999999,
      ),
    ];
  }

  <<DataProvider('provideWriteCases')>>
  public function testWrite(int $chunk_size): void {
    $buffer = new TChunkedMemoryBuffer($chunk_size);
    $buffer->write(self::TEST_DATA);

    foreach ($buffer->getChunks() as $buf) {
      expect(Str\length($buf))->toBeLessThanOrEqualTo($chunk_size);
    }
    $out = Str\join($buffer->getChunks(), "");
    expect($out)->toBePHPEqual(self::TEST_DATA);
  }

  public function testWriteRandom(): void {
    $chunk_size = 50;
    $buffer = new TChunkedMemoryBuffer($chunk_size);
    $random = new PseudoRandomNumberGenerator(0);
    $i = 0;
    while ($i < Str\length(self::TEST_DATA)) {
      $read_len = $random->next(0, 25);
      $data = Str\slice(self::TEST_DATA, $i, $read_len);
      $buffer->write($data);
      $i += $read_len;
    }

    foreach ($buffer->getChunks() as $buf) {
      expect(Str\length($buf))->toBeLessThanOrEqualTo($chunk_size);
    }
    $out = Str\join($buffer->getChunks(), "");
    expect($out)->toBePHPEqual(self::TEST_DATA);
  }
}

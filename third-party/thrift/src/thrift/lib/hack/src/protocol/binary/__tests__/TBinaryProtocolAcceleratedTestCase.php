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
final class TBinaryProtocolAcceleratedTestCase extends WWWTest {

  use ClassLevelTest;

  public static function providerTBinaryProtocolAccelerated(
  ): vec<(KeyedContainer<int, LogEntry>)> {
    return vec[
      tuple(vec[
        LogEntry::fromShape(shape(
          'category' => 'test',
          'message' => '['.Str\repeat('1', 10000).']',
        )),
        LogEntry::fromShape(shape(
          'category' => 'test',
          'message' => '['.Str\repeat('2', 10000).']',
        )),
        LogEntry::fromShape(shape('category' => 'test', 'message' => '[3]')),
      ]),
      tuple(vec[
        LogEntry::fromShape(shape('category' => 'test', 'message' => '[1]')),
        LogEntry::fromShape(shape('category' => 'test', 'message' => '[2]')),
        LogEntry::fromShape(shape('category' => 'test', 'message' => '[3]')),
        LogEntry::fromShape(shape('category' => 'test', 'message' => '[4]')),
      ]),
    ];
  }

  <<
    DataProvider('providerTBinaryProtocolAccelerated'),
    JKBoolDataProvider(
      'thrift/hack:use_common_rpc_helpers',
      'thrift/hack:use_struct_to_string_extensions_in_rpc',
    ),
  >>
  public async function testTBinaryProtocolAccelerated(
    KeyedContainer<int, LogEntry> $messages,
  ): Awaitable<void> {
    $buffer1 = new TMemoryBuffer();
    $buffer2 = new TMemoryBuffer();

    $transport1 = new TFramedTransport($buffer1);
    $transport2 = new TFramedTransport($buffer2);

    $protocol1 = new TBinaryProtocolUnaccelerated($transport1);
    $protocol2 = new TBinaryProtocolAccelerated($transport2);

    $client1 = new scribeClient($protocol1);
    $client2 = new scribeClient($protocol2);
    ThriftClientTestUtils::mockRPCResponse(ResultCode::OK, scribeClient::class);
    await $client1->Log($messages);
    await $client2->Log($messages);

    $istr = 'TBinaryProtocolUnaccelerated and TBinaryProtocolAccelerated';
    expect(Str\length($buffer2->getBuffer()))->toBePHPEqual(
      Str\length($buffer1->getBuffer()),
      '%s',
      Str\format('%s write the same amount', $istr),
    );
    expect($buffer2->getBuffer())->toBePHPEqual(
      $buffer1->getBuffer(),
      '%s',
      Str\format('%s write the same content', $istr),
    );
  }

  <<
    DataProvider('providerTBinaryProtocolAccelerated'),
    JKBoolDataProvider(
      'thrift/hack:use_common_rpc_helpers',
      'thrift/hack:use_struct_to_string_extensions_in_rpc',
    ),
  >>
  public async function testTBinaryProtocolAcceleratedBufferedTransport(
    KeyedContainer<int, LogEntry> $messages,
  ): Awaitable<void> {
    $buffer1 = new TMemoryBuffer();
    $buffer2 = new TMemoryBuffer();

    $transport1 = new TBufferedTransport<TMemoryBuffer>($buffer1);
    $transport2 = new TBufferedTransport<TMemoryBuffer>($buffer2);

    $protocol1 = new TBinaryProtocolUnaccelerated($transport1);
    $protocol2 = new TBinaryProtocolAccelerated($transport2);

    $client1 = new scribeClient($protocol1);
    $client2 = new scribeClient($protocol2);

    ThriftClientTestUtils::mockRPCResponse(ResultCode::OK, scribeClient::class);
    await $client1->Log($messages);
    await $client2->Log($messages);

    $istr = 'TBinaryProtocolUnaccelerated and TBinaryProtocolAccelerated';
    expect(Str\length($buffer2->getBuffer()))->toBePHPEqual(
      Str\length($buffer1->getBuffer()),
      '%s',
      Str\format('%s write the same amount', $istr),
    );
    expect($buffer2->getBuffer())->toBePHPEqual(
      $buffer1->getBuffer(),
      '%s',
      Str\format('%s write the same content', $istr),
    );
  }

  public function testWriteReadRPCStruct(): void {
    $buffer = new TMemoryBuffer();
    $prot = new TBinaryProtocolAccelerated($buffer);

    $input =
      LogEntry::fromShape(shape('category' => 'test', 'message' => 'hello'));
    $prot->writeRPCStruct($input);
    $prot->getTransport()->flush();

    $output = $prot->readRPCStruct(LogEntry::class);
    expect($output)->toBePHPEqual($input);
  }
}

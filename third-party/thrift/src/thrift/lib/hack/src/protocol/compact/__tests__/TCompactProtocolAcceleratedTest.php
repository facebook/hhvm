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

/**
 * Test TCompactProtocolAccelerated against TCompactProtocol
 */
<<Oncalls('thrift')>>
final class TCompactProtocolAcceleratedTest extends WWWTest {
  use ClassLevelTest;
  private function getReferenceProtocol(): TCompactProtocolUnaccelerated {
    return new TCompactProtocolUnaccelerated(new TMemoryBuffer());
  }

  private function getAcceleratedProtocol(): TCompactProtocolAccelerated {
    return new TCompactProtocolAccelerated(new TMemoryBuffer());
  }

  private function getThriftClient(
    bool $use_accelerated_protocol,
    bool $throw_exn = false,
  ): CompactTestServiceClient {
    list($client, $_) = LocalThriftConnection::setup<
      CompactTestServiceClient,
      TCompactProtocolAcceleratedTestServer,
    >(($server) ==> {
      $server->throwExn = $throw_exn;
      $server->useAcceleratedProtocol = $use_accelerated_protocol;
    });
    return $client;
  }

  private function getTestData(
    classname<CompactTestStruct> $class = CompactTestStruct::class,
  ): CompactTestStruct {
    $ret = new $class();

    $ret->i1 = 42;
    $ret->i2 = 100;
    $ret->i3 = -100;

    $ret->b1 = true;
    $ret->b2 = false;

    $ret->doubles = vec[21.68, -1.04, 2938448.6, 0.0, -1932820.90837];

    $ret->ints = dict[
      201938 => true,
      -19283 => true,
      2147483647 => true,
      -2147483647 => true,
      4611686016279904257 => true,
    ];

    $ret->m1 = dict[
      'one thousand' => 1000,
      'negative two hundred and six' => -206,
      'the lonliest number' => 1,
    ];

    $ret->m2 = dict[
      -2 => vec['negative', 'two'],
      64 => vec['a binary', 'number!'],
      31337 => vec['leet', 'haxor'],
      0 => vec['llama', 'llama', 'duck'],
      -19883 => vec[],
    ];

    $small1 = CompactTestStructSmall::withDefaultValues();
    $small1->bools = vec[true, true, false, true];
    $small1->ints = vec[
      1,
      2,
      3,
      4,
      5,
      6,
      7,
      8,
      9,
      10,
      11,
      12,
      13,
      14,
      15,
      16,
      17,
    ];

    $small2 = CompactTestStructSmall::withDefaultValues();
    $small2->bools = vec[false, true, false, true];
    $small2->ints = vec[
      -1,
      2,
      -3,
      4,
      -5,
      6,
      -7,
      8,
      -9,
      10,
      -11,
      12,
      -13,
      14,
      -15,
      16,
    ];

    $ret->structs = vec[$small1, $small2];

    $ret->s = 'Welcome to Clowntown.';

    return $ret;
  }

  private async function genEncode(TProtocol $prot): Awaitable<string> {
    ThriftClientTestUtils::mockRPCResponse(
      CompactTestStruct::withDefaultValues(),
      CompactTestServiceClient::class,
    );
    $client = new CompactTestServiceClient($prot);
    await $client->test($this->getTestData());
    // TODO (partisan): Migrate to a singleton serializer
    return ($prot->getTransport() as TMemoryBuffer)->getBuffer();
  }

  <<JKBoolDataProvider('thrift/hack:use_common_rpc_helpers')>>
  public async function testEncode(): Awaitable<void> {
    $reference = await $this->genEncode($this->getReferenceProtocol());
    $accelerated = await $this->genEncode($this->getAcceleratedProtocol());

    expect(non_crypto_md5($accelerated))->toBePHPEqual(
      non_crypto_md5($reference),
    );
  }
  private async function genRoundTrip(
    bool $use_accelerated_protocol,
    bool $throw_exn = false,
  ): Awaitable<CompactTestStruct> {
    $client = $this->getThriftClient($use_accelerated_protocol, $throw_exn);
    return await $client->test($this->getTestData());
  }

  <<JKBoolDataProvider('thrift/hack:use_common_rpc_helpers')>>
  public async function testgenRoundTrip(): Awaitable<void> {
    $reference = await $this->genRoundTrip(false);
    $accelerated = await $this->genRoundTrip(true);

    expect($accelerated)->toBePHPEqual($reference);
    expect($accelerated)->toBePHPEqual($this->getTestData());
  }

  <<JKBoolDataProvider('thrift/hack:use_common_rpc_helpers')>>
  public async function testExn(): Awaitable<void> {
    $reference = 'reference did not throw';
    try {
      await $this->genRoundTrip(false, true);
    } catch (CompactTestExn $e) {
      $reference = $e->t;
    }

    $accelerated = 'accelerated did not throw';
    try {
      await $this->genRoundTrip(true, true);
    } catch (CompactTestExn $e) {
      $accelerated = $e->t;
    }

    expect($accelerated)->toBePHPEqual($reference);
    expect($this->getTestData())->toBePHPEqual($accelerated);
  }

  public async function testSkip(): Awaitable<void> {
    $client = $this->getThriftClient(true);
    $output = await $client->test(
      $this->getTestData(CompactTestStructForSkipTest::class),
    );
    $full = $this->getTestData();

    expect($output->i1)->toBePHPEqual($full->i1);
    expect($output->i2)->toBeNull();
    expect($output->i3)->toBeNull();
    expect($output->b1)->toBeNull();
    expect($output->b2)->toBeNull();
    expect($output->doubles)->toBeNull();
    expect($output->ints)->toBeNull();
    expect($output->m1)->toBeNull();
    expect($output->m2)->toBePHPEqual($full->m2);
    expect($output->structs)->toBeNull();
    expect($output->s)->toBePHPEqual($full->s);
  }

  public function testWriteReadRPCStruct(): void {
    $buffer = new TMemoryBuffer();
    $prot = new TCompactProtocolAccelerated($buffer);

    $input = $this->getTestData();
    $prot->writeRPCStruct($input);
    $prot->getTransport()->flush();

    $output = $prot->readRPCStruct(CompactTestStruct::class);
    expect($output)->toBePHPEqual($input);
  }
}

<<Oncalls('thrift_hack')>>
final class TCompactProtocolAcceleratedTestServer extends METAThriftServer {
  const type TProcessor = CompactTestServiceAsyncProcessor;
  public bool $throwExn = false;
  public bool $useAcceleratedProtocol = true;

  <<__Override>>
  protected function getProcessor(): CompactTestServiceAsyncProcessor {
    return new CompactTestServiceAsyncProcessor(
      new CompactTestService($this->throwExn),
    );
  }

  <<__Override>>
  protected function useAcceleratedProtocol(): bool {
    return $this->useAcceleratedProtocol;
  }

  <<__Override>>
  protected function getProtocolType()[]: ThriftServerProtocol {
    return ThriftServerProtocol::COMPACT;
  }

  <<__Override>>
  public static function getService(): string {
    return 'dummy';
  }
}

final class CompactTestService implements CompactTestServiceAsyncIf {
  public function __construct(private bool $throwExn = false) {}
  public async function test(
    ?CompactTestStruct $t,
  ): Awaitable<CompactTestStruct> {
    if ($this->throwExn) {
      throw CompactTestExn::fromShape(shape('t' => $t));
    }
    return nullthrows($t, 'Got unexpected null');
  }
}

final class CompactTestStructForSkipTest extends CompactTestStruct {

  const ThriftStructTypes::TSpec SPEC = dict[
    1 => shape(
      'var' => 'i1',
      'type' => TType::I32,
    ),
    9 => shape(
      'var' => 'm2',
      'type' => TType::MAP,
      'ktype' => TType::I32,
      'vtype' => TType::LST,
      'key' => shape(
        'type' => TType::I32,
      ),
      'val' => shape(
        'type' => TType::LST,
        'etype' => TType::STRING,
        'elem' => shape(
          'type' => TType::STRING,
        ),
        'format' => 'array',
      ),
      'format' => 'array',
    ),
    3681 => shape(
      'var' => 's',
      'type' => TType::STRING,
    ),
  ];
}

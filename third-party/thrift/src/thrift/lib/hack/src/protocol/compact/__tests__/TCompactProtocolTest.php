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
 * Test TCompactProtocol
 *
 */
<<Oncalls('thrift')>>
final class TCompactProtocolTest extends WWWTest {
  public function testSimple(): void {
    $prot = new TCompactProtocolUnaccelerated(new TMemoryBuffer());

    $kudo = thriftshim_Kudo::withDefaultValues();
    $kudo->is_awesome = true;
    $kudo->awesome_score = 127;
    $kudo->write($prot);
    $kudoIn = thriftshim_Kudo::withDefaultValues();
    $kudoIn->read($prot);
    expect($kudo)->toBePHPEqual($kudoIn);

    $foobar = thriftshim_FooBar::withDefaultValues();
    $foobar->foo = Vector {4.5, -2.3};
    $foobar->bar = "Test";
    $foobar->write($prot);
    $foobarIn = thriftshim_FooBar::withDefaultValues();
    $foobarIn->read($prot);
    expect($foobar)->toBePHPEqual($foobarIn);
  }

  public function testI64(): void {
    $prot = new TCompactProtocolUnaccelerated(new TMemoryBuffer());

    $pokemon = thriftshim_Pokemon::withDefaultValues();
    $pokemon->charmander = 43;
    $pokemon->charmeleon = 107373182;
    $pokemon->charizard = HH\FIXME\UNSAFE_CAST<num, int>(
      0 - PHP\pow(2, 63),
      'FIXME[4110] Exposed by typing PHP\pow',
    ); // min i64
    $pokemon->write($prot);
    $pokemonIn = thriftshim_Pokemon::withDefaultValues();
    $pokemonIn->read($prot);
    expect($pokemon)->toBePHPEqual($pokemonIn);

    $num = PHP\pow(2, 62) + (PHP\pow(2, 62) - 1);
    $pokemon->charizard = HH\FIXME\UNSAFE_CAST<num, int>(
      $num,
      'FIXME[4110] Exposed by typing PHP\pow',
    );
    $pokemon->write($prot);
    $pokemonIn = thriftshim_Pokemon::withDefaultValues();
    $pokemonIn->read($prot);
    expect($pokemon)->toBePHPEqual($pokemonIn);

    $num = PHP\pow(2, 30);
    $pokemon->charizard = HH\FIXME\UNSAFE_CAST<num, int>(
      $num,
      'FIXME[4110] Exposed by typing PHP\pow',
    );
    $pokemon->write($prot);
    $pokemonIn = thriftshim_Pokemon::withDefaultValues();
    $pokemonIn->read($prot);
    expect($pokemon)->toBePHPEqual($pokemonIn);
  }

  public function testMessage(): void {
    // Test whole message serialization
    $prot = new TCompactProtocolUnaccelerated(new TMemoryBuffer());

    $pokemon = thriftshim_Pokemon::withDefaultValues();
    $pokemon->charmander = 43;
    $pokemon->charmeleon = 107373182;
    $pokemon->charizard = HH\FIXME\UNSAFE_CAST<num, int>(
      0 - PHP\pow(2, 63),
      'FIXME[4110] Exposed by typing PHP\pow',
    ); // min i64

    $prot->writeMessageBegin('getPokemon', TMessageType::CALL, 1);
    $pokemon->write($prot);
    $prot->writeMessageEnd();
    // TODO (partisan): Migrate to a singleton serializer
    $prot->getTransport()->flush();

    $rseqid = 0;
    $fname = '';
    $mtype = -1;
    $prot->readMessageBegin(inout $fname, inout $mtype, inout $rseqid);
    expect('getPokemon')->toBePHPEqual($fname);
    expect(TMessageType::CALL)->toBePHPEqual($mtype);
    expect($rseqid)->toBePHPEqual(1);

    $pokemonIn = thriftshim_Pokemon::withDefaultValues();
    $pokemonIn->read($prot);
    expect($pokemon)->toBePHPEqual($pokemonIn);

    $prot->readMessageEnd();
  }
}

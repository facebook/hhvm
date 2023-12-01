<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\{Math, Str};
use const HH\Lib\_Private\ALPHABET_ALPHANUMERIC;
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\{DataProvider, HackTest};

// FB likes to be explicit about md5() being unsuitable for crypto, and
// our usual trivial wrapper isn't available in open source.
use function md5 as non_crypto_md5;

final class MathComputeTest extends HackTest {
  public static function provideTestAbs(): varray<mixed> {
    return vec[
      tuple(-1, 1),
      tuple(1, 1),
      tuple(-7.3, 7.3),
      tuple(7.3, 7.3),
      tuple(0, 0),
      tuple(0.0, 0.0),
    ];
  }

  <<DataProvider('provideTestAbs')>>
  public function testAbs(num $number, num $expected): void {
    expect(Math\abs($number))->toEqual($expected);
  }

  public static function provideTestBaseConvertBijection(): varray<mixed> {
    return vec[
      tuple('a', 16, 10, '10'),
      tuple('a', 16, 2, '1010'),
      tuple('50f', 16, 36, 'zz'),
      tuple('1295', 10, 36, 'zz'),
      tuple('5d41402abc4b2a76b9719d911017c592', 16, 36, '5ir3t0ozoelrnauhrwyu1xfgy'),
      tuple('a37334', 16, 2, '101000110111001100110100'),
      tuple('120321300231', 4, 8, '30716055'),
      tuple('10100101000100010', 2, 36, '1t7m'),
      tuple('101010100101010101', 2, 36, '3ql1'),
      tuple('10101010101110100101', 2, 36, 'ezl1'),
      tuple('11010101', 2, 36, '5x'),
      tuple('0', 2, 36, '0'),
      tuple('1', 2, 36, '1'),
      tuple('1100101010', 2, 36, 'mi'),
      tuple('100010100010101', 2, 36, 'dn9'),
      tuple('101011010101', 2, 36, '251'),
      tuple(
        '56d0350026a4b57b34a882fc109b7a0a',
        16,
        20,
        '22jf2iicg82eh3ihigcc42ej103fai',
      ),
      tuple(
        '56d0350026a4b57b34a882fc109b7a0a',
        16,
        8,
        '1266403240011522265366322504057602046675012',
      ),
      tuple(
        '56d0350026a4b57b34a882fc109b7a0a',
        16,
        3,
        '210002010120011110111011221010111202002201210222222010110200222001121'.
          '21120102220',
      ),
      tuple(
        '56d0350026a4b57b34a882fc109b7a0a',
        16,
        2,
        '101011011010000001101010000000000100110101001001011010101111011001101'.
          '0010101000100000101111110000010000100110110111101000001010',
      ),
    ];
  }

  <<DataProvider('provideTestBaseConvertBijection')>>
  public function testBaseConvertBijection(
    string $from_value,
    int $from_base,
    int $to_base,
    string $to_value,
  ): void {
    expect(Math\base_convert($from_value, $from_base, $to_base))
      ->toEqual($to_value);
    expect(Math\base_convert($to_value, $to_base, $from_base))
      ->toEqual($from_value);
  }

  public function testBaseConvertIdentity(): void {
    $iters = 100;
    for ($i = 0; $i < $iters; $i++) {
      $length = mt_rand(1, 200);
      $base = mt_rand(2, 36);
      $alphabet = Str\slice(ALPHABET_ALPHANUMERIC, 0, $base);
      $random_string = '';
      for ($i = 0; $i < $length; ++$i) {
        $random_string .= Str\slice($alphabet, mt_rand($i === 0 && $length > 1 ? 1 : 0, $base - 1), 1);
      }
      expect(Math\base_convert($random_string, $base, $base))
        ->toEqual($random_string);
    }
  }

  public static function provideTestBaseConvertOneWay(): varray<mixed> {
    return vec[
      tuple('00000a', 16, 10, '10'),
      tuple('00000a', 16, 2, '1010'),
      tuple('50F', 16, 36, 'zz'),
      tuple('01295', 10, 36, 'zz'),
      tuple('05d41402abc4b2a76b9719d911017c592', 16, 36, '5ir3t0ozoelrnauhrwyu1xfgy'),
      tuple('A37334', 16, 2, '101000110111001100110100'),
      tuple('0120321300231', 4, 8, '30716055'),
      tuple('010100101000100010', 2, 36, '1t7m'),
      tuple('0101010100101010101', 2, 36, '3ql1'),
      tuple('010101010101110100101', 2, 36, 'ezl1'),
      tuple('011010101', 2, 36, '5x'),
      tuple('000', 2, 36, '0'),
      tuple('01', 2, 36, '1'),
      tuple('01100101010', 2, 36, 'mi'),
      tuple('0100010100010101', 2, 36, 'dn9'),
      tuple('0101011010101', 2, 36, '251'),
      tuple(
        '0'.Str\uppercase('56d0350026a4b57b34a882fc109b7a0a'),
        16,
        20,
        '22jf2iicg82eh3ihigcc42ej103fai',
      ),
      tuple(
        '0'.Str\uppercase('56d0350026a4b57b34a882fc109b7a0a'),
        16,
        8,
        '1266403240011522265366322504057602046675012',
      ),
      tuple(
        '0'.Str\uppercase('56d0350026a4b57b34a882fc109b7a0a'),
        16,
        3,
        '210002010120011110111011221010111202002201210222222010110200222001121'.
          '21120102220',
      ),
      tuple(
        '0'.Str\uppercase('56d0350026a4b57b34a882fc109b7a0a'),
        16,
        2,
        '101011011010000001101010000000000100110101001001011010101111011001101'.
          '0010101000100000101111110000010000100110110111101000001010',
      ),
    ];
  }

  <<DataProvider('provideTestBaseConvertOneWay')>>
  public function testBaseConvertOneWay(
    string $value,
    int $from_base,
    int $to_base,
    string $expected,
  ): void {
    expect(Math\base_convert($value, $from_base, $to_base))
      ->toEqual($expected);
  }

  public static function provideTestBaseConvertException(): varray<mixed> {
    return vec[
      // empty string
      tuple('', 2, 16),
      // invalid base
      tuple('a', 0, 2),
      tuple('a', 2, 1),
      // invalid digits
      tuple('a', 10, 2),
      tuple('119z', 10, 36),
    ];
  }

  <<DataProvider('provideTestBaseConvertException')>>
  public function testBaseConvertException(
    string $value,
    int $from_base,
    int $to_base,
  ): void {
    expect(() ==> Math\base_convert($value, $from_base, $to_base))
      ->toThrow(InvariantException::class);
  }

  public static function provideTestCeil(): varray<mixed> {
    return vec[
      tuple(3.5, 4.0),
      tuple(4, 4.0),
      tuple(-3.5, -3.0),
      tuple(-3.0, -3.0),
      tuple(0.0, 0.0),
    ];
  }

  <<DataProvider('provideTestCeil')>>
  public function testCeil(num $value, float $expected): void {
    expect(Math\ceil($value))->toEqual($expected);
  }

  public static function provideTestCos(): varray<mixed> {
    return vec[
      tuple(0.0, 1.0),
      tuple(M_PI_2, -3.4914813388431e-15),
      tuple(-M_PI_2, -3.4914813388431e-15),
      tuple(M_PI, -1.0),
      tuple(-M_PI, -1.0),
      tuple(3 * M_PI_2, 1.0474444016529e-14),
      tuple(M_PI / 6.0, M_SQRT3 / 2.0),
      tuple(M_PI / 3.0, 0.5),
      tuple(M_PI_4, M_SQRT2 / 2.0),
    ];
  }

  <<DataProvider('provideTestCos')>>
  public function testCos(num $arg, float $expected): void {
    $actual = Math\cos($arg);
    expect($actual)->toAlmostEqual($expected);
  }

  public static function provideTestExp(): varray<mixed> {
    return vec[
      tuple(-1.0, 1.0 / M_E),
      tuple(0.0, 1.0),
      tuple(1.0, M_E),
      tuple(2.0, M_E ** 2),
    ];
  }

  <<DataProvider('provideTestExp')>>
  public function testExp(num $arg, float $expected): void {
    $actual = Math\exp($arg);
    expect($actual)->toAlmostEqual($expected);
  }

  public static function provideTestFloor(): varray<mixed> {
    return vec[
      tuple(3.5, 3.0),
      tuple(4, 4.0),
      tuple(-3.5, -4.0),
      tuple(-3.0, -3.0),
      tuple(0.0, 0.0),
    ];
  }

  <<DataProvider('provideTestFloor')>>
  public function testFloor(num $value, float $expected): void {
    expect(Math\floor($value))->toEqual($expected);
  }

  public static function provideTestFromBase(): varray<mixed> {
    $tuples = vec[
      tuple('4d2', 16, 1234),
      tuple('2322', 8, 1234),
      tuple('10011010010', 2, 1234),
      tuple('AZikM', 36, 18453190),
      tuple('33CCFF', 16, 3394815),
    ];
    for ($n = 2; $n < 36; $n++) {
      $tuples[] = tuple(
        Math\base_convert((string)PHP_INT_MAX, 10, $n),
        $n,
        PHP_INT_MAX,
      );
    }
    return $tuples;
  }

  <<DataProvider('provideTestFromBase')>>
  public function testFromBase(
    string $number,
    int $from_base,
    int $expected,
  ): void {
    expect(Math\from_base($number, $from_base))->toEqual($expected);
  }

  public static function provideTestFromBaseException(): varray<mixed> {
    $tuples = vec[
      // invalid base
      tuple('1234', 0),
      tuple('1234', -1),
      tuple('1234', 37),
      tuple('1234', 100),
      // invalid characters
      tuple('abcd', 10),
      tuple('9999', 8),
      tuple('1234', 2),
      tuple('abcdefg', 16),
      tuple('-9223372036854775809', 10),
      tuple('-2322', 8, -1234),
      // integer overflow
      tuple('8f00000000000000', 16),
      tuple('17f00000000000000', 16),
    ];
    // overflow (PHP_INT_MAX + 1) in all bases
    for ($n = 2; $n < 36; $n++) {
      $tuples[] = tuple(
        Math\base_convert("9223372036854775808", 10, $n),
        $n,
      );
    }
    return $tuples;
  }

  <<DataProvider('provideTestFromBaseException')>>
  public function testFromBaseException(
    string $number,
    int $from_base,
    mixed $_FIXME_too_many_args_from_DataProvider = null,
  ): void {
    expect(() ==> Math\from_base($number, $from_base))
      ->toThrow(InvariantException::class);
  }

  public static function provideTestIntDiv(): varray<mixed> {
    return vec[
      tuple(1, 2, 0),
      tuple(2, 1, 2),
      tuple(-1, 2, 0),
      tuple(-2, 1, -2),
      tuple(-5, 2, -2),
      tuple(-5, -2, 2),
    ];
  }

  <<DataProvider('provideTestIntDiv')>>
  public function testIntDiv(
    int $numerator,
    int $denominator,
    int $expected,
  ): void {
    expect(Math\int_div($numerator, $denominator))->toEqual($expected);
  }

  public static function provideTestIntDivException(): varray<mixed> {
    return vec[
      tuple(-1, 0),
      tuple(0, 0),
      tuple(1, 0),
    ];
  }

  <<DataProvider('provideTestIntDivException')>>
  public function testIntDivException(int $numerator, int $denominator): void {
    expect(() ==> Math\int_div($numerator, $denominator))
      ->toThrow(DivisionByZeroException::class);
  }

  public static function provideTestLog(): varray<mixed> {
    return vec[
      tuple(M_E),
      tuple(10),
      tuple(2),
    ];
  }

  <<DataProvider('provideTestLog')>>
  public function testLog(num $base): void {
    for ($exp = 0.5; $exp <= 10.0; $exp += 0.5) {
      $actual = Math\log($base ** $exp, $base);
      expect($actual)->toAlmostEqual($exp);
    }
  }

  public static function provideTestLogNoBase(): varray<mixed> {
    return vec[
      tuple(0.1),
      tuple(3.6),
      tuple(M_E),
      tuple(100.0),
    ];
  }

  <<DataProvider('provideTestLogNoBase')>>
  public function testLogNoBase(num $base): void {
    expect(Math\log($base))->toEqual(log((float) $base));
    expect(Math\log($base, null))->toEqual(log((float) $base));
  }

  public function testLogException(): void {
    expect(() ==> Math\log(-1, 2))->toThrow(InvariantException::class);
    expect(() ==> Math\log(3, 0))->toThrow(InvariantException::class);
    expect(() ==> Math\log(3, -5))->toThrow(InvariantException::class);
    expect(() ==> Math\log(3, 1))->toThrow(InvariantException::class);
  }

  public static function provideTestRound(): varray<mixed> {
    return vec[
      tuple(3.5, 0, 4.0),
      tuple(4, 0, 4.0),
      tuple(-3.5, 0, -4.0),
      tuple(-3.4, 0, -3.0),
      tuple(-3.0, 0, -3.0),
      tuple(0.0, 0, 0.0),
      tuple(0.25, 1, 0.3),
      tuple(-0.25, 1, -0.3),
      tuple(0.24, 1, 0.2),
      tuple(-0.24, 1, -0.2),
      tuple(549.375, 2, 549.38),
      tuple(549.375, 1, 549.4),
      tuple(549.375, 0, 549.0),
      tuple(549.375, -1, 550.0),
      tuple(549.375, -2, 500.0),
      tuple(549.375, -3, 1000.0),
      tuple(549.375, -4, 0.0),
    ];
  }

  <<DataProvider('provideTestRound')>>
  public function testRound(
    num $value,
    int $precision,
    float $expected,
  ): void {
    expect(Math\round($value, $precision))->toEqual($expected);
  }

  public static function provideTestSin(): varray<mixed> {
    return vec[
      tuple(0.0, 0.0),
      tuple(M_PI_2, 1.0),
      tuple(-M_PI_2, -1.0),
      tuple(M_PI, -6.9829626776863e-15),
      tuple(-M_PI, 6.9829626776863e-15),
      tuple(3 * M_PI_2, -1.0),
      tuple(M_PI / 6.0, 0.5),
      tuple(M_PI / 3.0, M_SQRT3 / 2.0),
      tuple(M_PI_4, M_SQRT2 / 2.0),
    ];
  }

  <<DataProvider('provideTestSin')>>
  public function testSin(num $arg, float $expected): void {
    $actual = Math\sin($arg);
    expect($actual)->toAlmostEqual($expected);
  }

  public static function provideTestSqrt(): varray<mixed> {
    return vec[
      tuple(16.0, 4.0),
      tuple(2, M_SQRT2),
      tuple(3, M_SQRT3),
      tuple(0, 0.0),
      tuple(1, 1.0),
    ];
  }

  <<DataProvider('provideTestSqrt')>>
  public function testSqrt(num $arg, float $expected): void {
    $actual = Math\sqrt($arg);
    expect($actual)->toAlmostEqual($expected);
  }

  public function testSqrtException(): void {
    expect(() ==> Math\sqrt(-1))->toThrow(InvariantException::class);
  }

  public static function provideTestToBase(): varray<mixed> {
    $tuples = vec[
      tuple(1234, 16, '4d2'),
      tuple(1234, 8, '2322'),
      tuple(1234, 2, '10011010010'),
      tuple(0xdeadbeef, 16, 'deadbeef'),
      tuple(0xdeadbeef, 3, '100122100210211112102'),
      tuple(0xdeadbeef, 34, '2e7m43p'),
      tuple(PHP_INT_MAX, 10, '9223372036854775807'),
      tuple(PHP_INT_MAX, 2, Str\repeat('1', 63)),
      tuple(PHP_INT_MAX, 16, '7fffffffffffffff'),
      tuple(PHP_INT_MAX, 17, '33d3d8307b214008'),
      tuple(PHP_INT_MAX, 36, '1y2p0ij32e8e7'),
    ];
    for ($i = 2; $i <= 36; ++$i) {
      $tuples[] = tuple(0, $i, '0');
      $tuples[] = tuple(1, $i, '1');
    }
    return $tuples;
  }

  <<DataProvider('provideTestToBase')>>
  public function testToBase(
    int $number,
    int $to_base,
    string $expected,
  ): void {
    expect(Math\to_base($number, $to_base))->toEqual($expected);
  }

  public static function provideTestToBaseException(): varray<mixed> {
    return vec[
      tuple(1234, 0),
      tuple(1234, -1),
      tuple(1234, 37),
      tuple(1234, 100),
      tuple(-1234, 8),
      tuple(-1, -123),
      tuple(1234, 1),
    ];
  }

  <<DataProvider('provideTestToBaseException')>>
  public function testToBaseException(int $number, int $to_base): void {
    expect(() ==> Math\to_base($number, $to_base))
      ->toThrow(InvariantException::class);
  }
}

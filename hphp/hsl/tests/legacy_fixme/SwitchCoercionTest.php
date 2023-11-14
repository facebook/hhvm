<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\Legacy_FIXME;
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\{HackTest, CoercionsTestUtils, DataProvider};

final class SwitchCoercionTest extends HackTest {
  use CoercionsTestUtils;

  private function assert(
    (function(mixed): mixed) $switch,
    (function(): mixed) $phpism,
  ): void {
    $case = \HH\class_meth_get_method($switch).' '.$this->getDataLabel();
    try {
      $res = $switch($phpism());
    } catch (\Exception $e) {
      // handle closure numbers
      $msg = \HH\Lib\Regex\replace(
        $e->getMessage(),
        re"/CoercionsTestUtils::getData(#\d+)?;\d+/",
        'CoercionsTestUtils::getData',
      );
      self::assertUnchanged($msg, $case);
      return;
    }
    self::assertUnchanged(var_export($res, true), $case);
  }

  private function assertIntSwitch(
    (function(mixed): int) $switch,
    mixed $t,
    int $nonzero,
  ): void {
    $assert = $phpism ==> $this->assert($switch, $phpism);
    if ($t is ?arraykey) {
      $assert(() ==> Legacy_FIXME\optional_arraykey_to_int_cast_for_switch($t));
    }
    if ($t is ?num) {
      $assert(() ==> Legacy_FIXME\optional_num_to_int_cast_for_switch($t));
    }
    $assert(() ==> Legacy_FIXME\int_cast_for_switch($t, $nonzero));
  }

  public static function dataProvider(): vec<(mixed)> {
    return vec[
      tuple(-120),
      tuple(-1),
      tuple(0),
      tuple(1),
      tuple(2),
      tuple(3),
      tuple(4),
      tuple(5),
      tuple(0.0),
      tuple(1.0),
      tuple(2.0),
      tuple(2.2),
      tuple(3.0),
      tuple(3.21200),
      tuple(5.0),
      tuple(5.3920),
      tuple(5.5),
      tuple(5.5001),
      tuple(0x7fffffffffffffff),
      tuple(false),
      tuple(true),
      tuple(null),
      tuple(''),
      tuple('0'),
      tuple('0eab'),
      tuple('1'),
      tuple('1abc'),
      tuple('2'),
      tuple('2bananas'),
      tuple('3'),
      tuple('3a'),
      tuple('3abc'),
      tuple('4'),
      tuple('4abc'),
      tuple('0.0'),
      tuple('2.0'),
      tuple('2.0derp'),
      tuple('2.2'),
      tuple('3.0'),
      tuple('3.0herp'),
      tuple('4.0'),
      tuple('5.0'),
      tuple('5.3920'),
      tuple('5.5'),
      tuple('5.5001'),
      tuple('5'),
      tuple('baz'),
      tuple('blar'),
      tuple('foo'),
      tuple('jazz'),
      tuple(<xhpclass></xhpclass>),
      tuple(<xhpclass>bananas</xhpclass>),
      tuple(new SimpleXMLElement('<xhpclass/>')),
      tuple(new SimpleXMLElement('<xhpclass> bananas </xhpclass>')),
      tuple(new SwitchCoercionsTestC()),
      tuple(new SwitchCoercionsTestDateTime(4)),
      tuple(new SwitchCoercionsTestDateTime(0)),
      tuple(new SwitchCoercionsTestDateTime(-1)),
      tuple(new stdClass()),
      tuple(\imagecreate(10, 10)), // resource
      tuple(\log(0.0)),
      tuple(vec['foo', 'floo']),
      tuple(vec[]),
      tuple(Vector {'foo', 'floo'}),
      tuple(Vector {}),
    ];
  }

  <<DataProvider('dataProvider')>>
  public function testIntSwitch(mixed $thing): void {
    $this->assertIntSwitch(self::intSwitch1<>, $thing, -1);
    $this->assertIntSwitch(self::intSwitch2<>, $thing, 5);
  }

  private static function intSwitch1(mixed $foo): int {
    switch ($foo) {
      case 0:
        return 0;
      case -1:
        return -1;
      case 1:
        return 1;
      case 4:
        return 4;
      case 2:
        return 2;
      case 5:
        return 5;
      default:
        return 42;
    }
  }

  private static function intSwitch2(mixed $foo): int {
    switch ($foo) {
      case 5:
        return 5;
      case 1:
        return 1;
      case 0:
        return 0;
      default:
        return 42;
    }
  }

  const string BasicSwitchFirstCase = 'foo';

  <<DataProvider('dataProvider')>>
  public function testBasicStringSwitches(mixed $t): void {
    $assert = $phpism ==> $this->assert(self::basicStringSwitch<>, $phpism);
    $assert(
      () ==> Legacy_FIXME\string_cast_for_basic_switch(
        $t,
        self::BasicSwitchFirstCase,
      ),
    );
    $assert(
      () ==> Legacy_FIXME\string_cast_for_switch(
        $t,
        self::BasicSwitchFirstCase,
        self::BasicSwitchFirstCase,
      ),
    );
  }

  private static function basicStringSwitch(mixed $in): string {
    switch ($in) {
      case self::BasicSwitchFirstCase:
        return self::BasicSwitchFirstCase;
      case 'bar':
        return 'bar';
      case 'baz':
        return 'baz';
      case 'foo2':
        return 'foo2';
      case 'bar2':
        return 'bar2';
      case 'baz2':
        return 'baz2';
      case 'foo3':
        return 'foo3';
      case 'bar3':
        return 'bar3';
      case 'baz3':
        return 'baz3';
      default:
        return 'default';
    }
  }

  <<DataProvider('dataProvider')>>
  public function testStringSwitches(mixed $t): void {
    // truthy, zeroish, falsy, intish dict, floatish dict
    $this->assert(
      self::stringSwitch1<>,
      () ==> Legacy_FIXME\string_cast_for_switch(
        $t,
        '123',
        '0',
        '0',
        dict['123' => 123, '4abc' => 4, '4' => 4, '0' => 0],
      ),
    );
    $this->assert(
      self::stringSwitch2<>,
      () ==> Legacy_FIXME\string_cast_for_switch(
        $t,
        'foo',
        'foo',
        '0',
        dict['1' => 1, '2.0' => 2, '2ab' => 2, '0' => 0],
        dict['3.212' => 3.212],
      ),
    );
    $this->assert(
      self::stringSwitch3<>,
      () ==>
        Legacy_FIXME\string_cast_for_switch($t, null, '', '', dict['0' => 0]),
    );
    $this->assert(
      self::stringSwitch4<>,
      () ==> Legacy_FIXME\string_cast_for_switch(
        $t,
        '3.0abc',
        null,
        null,
        dict['3.0abc' => 3, '3.0' => 3, '3' => 3],
      ),
    );
    $this->assert(
      self::stringSwitch5<>,
      () ==> Legacy_FIXME\string_cast_for_switch($t, '5', '0', '0', dict[
        '5' => 5,
        '4' => 4,
        '0' => 0,
        '3' => 3,
        '8' => 8,
        '16' => 16,
        '32' => 32,
        '64' => 64,
      ]),
    );
  }

  private static function stringSwitch1(mixed $x): string {
    switch ($x) {
      case '123':
        return '123';
      case '4abc':
        return '4abc';
      case '4':
        return '4';
      case '0':
        return '0';
      case '':
        return '{empty str}';
      case 'Derp':
        return 'Derp';
      default:
        return 'default';
    }
    echo '\n';
  }

  private static function stringSwitch2(mixed $x): string {
    switch ($x) {
      case 'foo':
        return 'foo';
      case '1':
        return '1';
      case '2.0':
        return '2.0';
      case '2ab':
        return '2ab';
      case '3.212':
        return '3.212';
      case '0':
        return '0';
      case '':
        return '{empty str}';
      default:
        return 'default';
    }
    echo '\n';
  }

  private static function stringSwitch3(mixed $x): string {
    switch ($x) {
      case '':
        return '{empty str}';
      case '0':
        return '0';
      default:
        return 'default';
    }
    echo '\n';
  }

  private static function stringSwitch4(mixed $x): string {
    switch ($x) {
      case '3.0abc':
        return '3.0abc';
      case '3.0':
        return '3.0';
      case '3':
        return '3';
      default:
        return 'default';
    }
    echo '\n';
  }

  private static function stringSwitch5(mixed $x): string {
    switch ($x) {
      case '5':
        return '5';
      case '4':
        return '4';
      case '0':
        return '0';
      case '3':
        return '3';
      case '8':
        return '8';
      case '16':
        return '16';
      case '32':
        return '32';
      case '64':
        return '64';
      default:
        return 'default';
    }
  }
}

final class SwitchCoercionsTestC {}

class SwitchCoercionsTestDateTime implements DateTimeInterface {
  public function __construct(public int $timestamp) {
    $this->timestamp = $timestamp;
  }
  public function getTimestamp()[]: int {
    if ($this->timestamp <= 0) {
      throw new Exception('sneaky');
    }
    return $this->timestamp;
  }
  public function diff(mixed $dt, mixed $absolute = null): void {}
  public function format(mixed $format): void {}
  public function getTimezone(): int { return 0; }
  public function getOffset(): int { return 0; }
}

xhp class xhpclass {
 public function __construct(
    public darray<string,mixed> $a, // Attributes
    public varray<mixed> $c, // Children
    public string $f, // Filename
    public int $ln, // Line number
  ) {}
}

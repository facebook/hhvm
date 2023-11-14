<?hh

/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\__Private\MiniTest;

use namespace HH\Lib\{C, Str, Regex};

trait CoercionsTestUtils {
  use CodegenAssertUnchanged;

  <<__Override>>
  public static function beforeFirstTest(): void {
    self::setupForCodegen();
  }
  <<__Override>>
  public static function afterLastTest(): void {
    self::tearDownForCodegen();
  }

  private function baseTest(
    mixed $lhs,
    mixed $rhs,
    (function(mixed, mixed): mixed) $f,
  ): void {
    $fun = C\lastx(Str\split(\HH\fun_get_function($f), '\\'));
    $case = $fun.' '.$this->getDataLabel();
    try {
      $res = $f($lhs, $rhs);
    } catch (\Exception $e) {
      // handle closure numbers
      $msg = Regex\replace(
        $e->getMessage(),
        re"/CoercionsTestUtils::getData(#\d+)?/",
        'CoercionsTestUtils::getData',
      );
      self::assertUnchanged($msg, $case);
      return;
    }
    $res = $res is bool ? ($res ? 'true' : 'false') : (string)$res;
    self::assertUnchanged($res, $case);
  }

  public static function getData(
  ): dict<string, shape('lhs' => mixed, 'rhs' => mixed)> {
    $a1 = new CoercionsTestA();
    $a2 = new CoercionsTestA();
    $a3 = new CoercionsTestA(new CoercionsTestA());
    $a4 = new CoercionsTestA(new CoercionsTestA());
    $a5 = new CoercionsTestA('bar');
    $a6 = new CoercionsTestA(new CoercionsTestToString());
    $a7 = new CoercionsTestA(new CoercionsTestToString('bar'));
    $a8 = new CoercionsTestA(new CoercionsTestToString('99'));
    $a9 = new CoercionsTestA(99);
    $a11 = new CoercionsTestA(new CoercionsTestDateTime1(1000));
    $a12 = new CoercionsTestA(new CoercionsTestDateTime1(1000));
    $a13 = new CoercionsTestA(new CoercionsTestDateTime1(100));
    $a14 = new CoercionsTestA(new CoercionsTestDateTime1(-1));
    $a15 = new CoercionsTestA(new CoercionsTestDateTime2());
    $a16 = new CoercionsTestA(\NAN);
    $a17 = new CoercionsTestA(\NAN);

    $ad1 = new CoercionsTestA();
    /* HH_FIXME[4053] fact */
    $ad1->c = 999;
    $ad2 = new CoercionsTestA();
    /* HH_FIXME[4053] fact */
    $ad2->c = 999;
    $ad3 = new CoercionsTestA();
    /* HH_FIXME[4053] fact */
    $ad3->d = 1000;
    $ad4 = new CoercionsTestA();
    /* HH_FIXME[4053] fact */
    $ad4->c = 999;
    /* HH_FIXME[4053] fact */
    $ad4->d = 1001;
    $ad5 = new CoercionsTestA();
    /* HH_FIXME[4053] fact */
    $ad5->c = 999;
    /* HH_FIXME[4053] fact */
    $ad5->e = 1001;
    $ad6 = new CoercionsTestA();
    /* HH_FIXME[4053] fact */
    $ad6->e = 1000;
    /* HH_FIXME[4053] fact */
    $ad6->c = 998;

    $b1 = new CoercionsTestB();
    $c1 = new CoercionsTestC();

    $t1 = new CoercionsTestDateTime1(1000);
    $t2 = new CoercionsTestDateTime1(1000);
    $t3 = new CoercionsTestDateTime1(100);
    $t4 = new CoercionsTestDateTime1(-1);
    $t5 = new CoercionsTestDateTime2();

    $xml = \simplexml_load_string("<root />")->unknown;

    $v1 = Vector {0, 1, 2, 3, 4};
    $v2 = Vector {0, 1, 2, 3, 4};
    $v3 = Vector {5, 6, 7, 8, 9};
    $v4 = Vector {1};
    $v5 = Vector {'1'};
    $v6 = Vector {0, 1, 2, 4, 3};

    $iv1 = ImmVector {0, 1, 2, 3, 4};

    $s1 = Set {1, 2};
    $s2 = Set {2, 1};
    $s3 = Set {2, '1'};

    $m1 = Map {42 => 42};
    $m2 = Map {'42' => 42};
    $m3 = Map {42 => '42'};
    $m4 = Map {1 => 2, 3 => 4};
    $m5 = Map {3 => 4, 1 => 2};
    $m6 = Map {3 => 4, 1 => '2'};

    $m7 = Map {'a' => Vector {1}, 'b' => Vector {2}};
    $m8 = Map {'a' => Vector {1}, 'b' => Vector {3}};
    $m9 = Map {'a' => Vector {1}, 'c' => Vector {2}};
    $m10 = Map {0 => 1};

    $im1 = ImmMap {42 => 42};

    $p1 = Pair {'elem1', 'elem2'};
    $p2 = Pair {'elem1', 'elem2'};
    $p3 = Pair {'other1', 'other2'};
    $p4 = Pair {0, 1};
    $p5 = Pair {0, '1'};
    $p6 = Pair {1, 0};

    $clo1 = function() {
      return 0;
    };
    $clo2 = function() {
      return 0;
    };

    $fp = CoercionsTest_foo<>;
    $clsmeth = CoercionsTestBar::baz<>;
    $meth_caller = meth_caller(CoercionsTestBar::class, 'qux');

    $arr1 = vec[];
    $arr2 = vec[99];
    $arr3 = vec['foo'];
    $arr4 = vec['foo', 'bar'];
    $arr5 = vec['foo', 'bar'];
    $arr6 = vec['foo', 'baz'];
    $arr7 = vec[new CoercionsTestA(), new CoercionsTestA()];
    $arr8 = vec[new CoercionsTestA(), new CoercionsTestA()];
    $arr9 = vec[new CoercionsTestA(), new CoercionsTestC()];
    $arr10 = vec[new CoercionsTestDateTime1(1000)];
    $arr11 = vec[new CoercionsTestDateTime2()];
    $arr12 = vec[vec[1, 2], vec[1, 2, 3]];
    $arr13 = vec[vec[1, 2], vec[1, 2, 3]];
    $arr14 = vec[vec[1, 2], vec[99]];
    $arr15 = vec[Vector {0, 1, 2, 3, 4}, Vector {5, 6, 7, 8}];
    $arr16 = vec[Vector {0, 1, 2, 3, 4}, Vector {5, 6, 7, 8}];
    $arr17 = dict['key1' => 1, 'key2' => 2, 'key3' => 3];
    $arr18 = dict['key1' => 1, 'key2' => 2, 'key3' => 3];
    $arr19 = dict['key1' => 1, 'key2-other' => 2, 'key3' => 3];
    $arr20 = dict['key2' => 2, 'key3' => 3, 'key1' => 1];
    $arr21 = vec['baz', 'foo'];
    $arr22 = vec[$clo1];

    $vec1 = vec[];
    $vec2 = vec[1, 2];
    $vec3 = vec[1, 2, 3];
    $vec4 = vec[1, 2, 4];
    $vec5 = vec[new CoercionsTestToString('foo')];
    $vec7 = vec[new CoercionsTestA()];
    $vec8 = vec[new CoercionsTestA()];
    $vec9 = vec[4, 2, 1];
    $vec10 = vec[1, 2, 'value'];

    $dict1 = dict[];
    $dict2 = dict['a' => 0, 'b' => 1];
    $dict3 = dict['a' => 0, 'b' => 1, 'c' => 2];
    $dict4 = dict['a' => 0, 'b' => 1, 'c' => 3];
    $dict5 = dict['a' => 0, 'z' => 1, 'c' => 3];
    $dict6 = dict[0 => new CoercionsTestToString('foo')];
    $dict8 = dict[0 => new CoercionsTestA()];
    $dict9 = dict[0 => new CoercionsTestA()];
    $dict10 = dict['c' => 2, 'b' => 1, 'a' => 0];

    $ks1 = keyset[];
    $ks2 = keyset[1];
    $ks3 = keyset[1];
    $ks4 = keyset['a'];
    $ks5 = keyset['a'];
    $ks6 = keyset[1, 2, 3];
    $ks7 = keyset[3, 2, 1];
    $ks8 = keyset['a', 'b', 'c'];
    $ks9 = keyset['c', 'b', 'a'];
    $ks10 = keyset[1, 2, 4];
    $ks11 = keyset['a', 'b', 'd'];
    $ks12 = keyset[100, 2, 3];
    $ks13 = keyset['z', 'b', 'c'];

    $f1 = \imagecreate(10, 10);
    $f2 = \imagecreate(10, 10);
    $f3 = \imagecreate(1, 1);

    $data = dict[
      'null' => null,

      'false' => false,
      'true' => true,

      'int 0' => 0,
      'int 99' => 99,
      'int -1' => -1,

      'float 0' => 0.0,
      'double 99' => (float)99,
      'float 99' => 99.0,
      'float -1' => -1.0,
      'INF' => \INF,
      '-INF' => -\INF,
      'NAN' => \NAN,

      '""' => "",
      '"0"' => "0",
      '"99"' => "99",
      '"99x"' => "99x",
      '"-1"' => "-1",
      '"0.0"' => "0.0",
      '"99.0"' => "99.0",
      '"-1.0"' => "-1.0",
      '"foo"' => "foo",
      '"BAZ"' => "BAZ",

      'array arr1' => $arr1,
      'array arr2' => $arr2,
      'array arr3' => $arr3,
      'array arr4' => $arr4,
      'array arr5' => $arr5,
      'array arr6' => $arr6,
      'array arr7' => $arr7,
      'array arr8' => $arr8,
      'array arr9' => $arr9,
      'array arr10' => $arr10,
      'array arr11' => $arr11,
      'array arr12' => $arr12,
      'array arr13' => $arr13,
      'array arr14' => $arr14,
      'array arr15' => $arr15,
      'array arr16' => $arr16,
      'array arr17' => $arr17,
      'array arr18' => $arr18,
      'array arr19' => $arr19,
      'array arr20' => $arr20,
      'array arr21' => $arr21,
      'array arr22' => $arr22,

      'vec vec1' => $vec1,
      'vec vec2' => $vec2,
      'vec vec3' => $vec3,
      'vec vec4' => $vec4,
      'vec vec5' => $vec5,
      'vec vec7' => $vec7,
      'vec vec8' => $vec8,
      'vec vec9' => $vec9,
      'vec vec10' => $vec10,

      'dict dict1' => $dict1,
      'dict dict2' => $dict2,
      'dict dict3' => $dict3,
      'dict dict4' => $dict4,
      'dict dict5' => $dict5,
      'dict dict6' => $dict6,
      'dict dict8' => $dict8,
      'dict dict9' => $dict9,
      'dict dict10' => $dict10,

      'keyset ks1' => $ks1,
      'keyset ks2' => $ks2,
      'keyset ks3' => $ks3,
      'keyset ks4' => $ks4,
      'keyset ks5' => $ks5,
      'keyset ks6' => $ks6,
      'keyset ks7' => $ks7,
      'keyset ks8' => $ks8,
      'keyset ks9' => $ks9,
      'keyset ks10' => $ks10,
      'keyset ks11' => $ks11,
      'keyset ks12' => $ks12,
      'keyset ks13' => $ks13,

      'object a1' => $a1,
      'object a2' => $a2,
      'object a3' => $a3,
      'object a4' => $a4,
      'object a5' => $a5,
      'object a6' => $a6,
      'object a7' => $a7,
      'object a8' => $a8,
      'object a9' => $a9,
      'object a11' => $a11,
      'object a12' => $a12,
      'object a13' => $a13,
      'object a14' => $a14,
      'object a15' => $a15,
      'object a16' => $a16,
      'object a17' => $a17,
      'object ad1' => $ad1,
      'object ad2' => $ad2,
      'object ad3' => $ad3,
      'object ad4' => $ad4,
      'object ad5' => $ad5,
      'object ad6' => $ad6,

      'object b1' => $b1,
      'object c1' => $c1,

      'object s1' => $s1,
      'object s2' => $s2,
      'object s3' => $s3,

      'object t1' => $t1,
      'object t2' => $t2,
      'object t3' => $t3,
      'object t4' => $t4,
      'object t5' => $t5,
      'object xml' => $xml,

      'vector v1' => $v1,
      'vector v2' => $v2,
      'vector v3' => $v3,
      'vector v4' => $v4,
      'vector v5' => $v5,
      'vector v6' => $v6,

      'immvector iv1' => $iv1,

      'set s1' => $s1,
      'set s2' => $s2,
      'set s3' => $s3,

      'map m1' => $m1,
      'map m2' => $m2,
      'map m3' => $m3,
      'map m4' => $m4,
      'map m5' => $m5,
      'map m6' => $m6,
      'map m7' => $m7,
      'map m8' => $m8,
      'map m9' => $m9,
      'map m10' => $m10,

      'immmap im1' => $im1,

      'pair p1' => $p1,
      'pair p2' => $p2,
      'pair p3' => $p3,
      'pair p4' => $p4,
      'pair p5' => $p5,
      'pair p6' => $p6,

      'closure clo1' => $clo1,
      'closure clo2' => $clo2,

      'resource f1' => $f1,
      'resource f2' => $f2,
      'resource f3' => $f3,

      'func_ptr' => $fp,
      'clsmeth' => $clsmeth,
      'meth_caller' => $meth_caller,
    ];

    $result = dict[];
    foreach ($data as $k1 => $v1_) {
      foreach ($data as $k2 => $v2_) {
        $result[$k1.' and '.$k2] = shape('lhs' => $v1_, 'rhs' => $v2_);
      }
    }
    return $result;
  }
}

class CoercionsTestA {
  public mixed $a = 5;
  public function __construct(public mixed $b = 'foo') {}
}

class CoercionsTestB {
  public mixed $a = 5;
  public function __construct(public mixed $b = 'foo') {}
}

class CoercionsTestC {
  public mixed $a = 5;
}

class CoercionsTestDateTime1 implements \DateTimeInterface {
  public int $timestamp = 0;
  public function __construct(int $timestamp) {
    $this->timestamp = $timestamp;
  }
  public function getTimestamp()[]: int {
    if ($this->timestamp <= 0) {
      throw new \Exception('sneaky');
    }
    return $this->timestamp;
  }
  public function diff(mixed $dt, mixed $absolute = null): void {}
  public function format(mixed $format): void {}
  public function getTimezone(): void {}
  public function getOffset(): int {
    return 0;
  }
}

class CoercionsTestDateTime2 implements \DateTimeInterface {
  public function getTimestamp()[]: int {
    return 100;
  }
  public function diff(mixed $dt, mixed $absolute = null): void {}
  public function format(mixed $format): void {}
  public function getTimezone(): void {}
  public function getOffset(): int {
    return 0;
  }
}

function CoercionsTest_foo(): void {}

class CoercionsTestBar {
  public static function baz(): void {}
  public function qux(): void {}
}

class CoercionsTestToString {
  public function __construct(public string $str = 'foo') {}
  public function __toString(): string {
    return $this->str;
  }
}

<?php

class C1 {
  public static function __callStatic($fn, $args) {
    echo "C1::__callStatic\n";
    var_dump(isset($this));
    var_dump($fn, $args);
    echo "\n";
  }
}

function main1() {
  // FPushClsMethodD
  C1::__callStatic("a", "b", "c", "d");
  C1::foo("a", "b", "c", "d");

  // FPushClsMethod
  $cls = 'C1';
  $cls::__callStatic("a", "b", "c", "d");
  $cls::foo("a", "b", "c", "d");
  $fn = '__callStatic';
  C1::$fn("a", "b", "c", "d");
  $fn = 'foo';
  C1::$fn("a", "b", "c", "d");
  $fn = '__callStatic';
  $cls::$fn("a", "b", "c", "d");
  $fn = 'foo';
  $cls::$fn("a", "b", "c", "d");
}

main1();

class C2 {
  public static function __callStatic($fn, $args) {
    echo "C2::__callStatic\n";
    var_dump(isset($this));
    var_dump($fn, $args);
    echo "\n";
  }
  public function test() {
    // FPushClsMethodD
    C2::__callStatic("a", "b", "c", "d");
    C2::foo("a", "b", "c", "d");

    // FPushClsMethod
    $cls = 'C2';
    $cls::__callStatic("a", "b", "c", "d");
    $cls::foo("a", "b", "c", "d");
    $fn = '__callStatic';
    C2::$fn("a", "b", "c", "d");
    $fn = 'foo';
    C2::$fn("a", "b", "c", "d");
    $fn = '__callStatic';
    $cls::$fn("a", "b", "c", "d");
    $fn = 'foo';
    $cls::$fn("a", "b", "c", "d");

    // FPushClsMethodF
    self::__callStatic("a", "b", "c", "d");
    self::foo("a", "b", "c", "d");
  }
}
function main2() {
  $obj = new C2;
  $obj->test();
}
main2();

class B3 {
  public function __call($fn, $args) {
    echo "B3::__call\n";
    var_dump(isset($this));
    var_dump($fn, $args);
    echo "\n";
  }
  public static function __callStatic($fn, $args) {
    echo "B3::__callStatic\n";
    var_dump(isset($this));
    var_dump($fn, $args);
    echo "\n";
  }
}
class C3 extends B3 {
  public function test() {
    // FPushClsMethodD
    B3::__callStatic("a", "b", "c", "d");
    B3::foo("a", "b", "c", "d");

    // FPushClsMethod
    $cls = 'B3';
    $cls::__callStatic("a", "b", "c", "d");
    $cls::foo("a", "b", "c", "d");
    $fn = '__callStatic';
    B3::$fn("a", "b", "c", "d");
    $fn = 'foo';
    B3::$fn("a", "b", "c", "d");
    $fn = '__callStatic';
    $cls::$fn("a", "b", "c", "d");
    $fn = 'foo';
    $cls::$fn("a", "b", "c", "d");

    // FPushClsMethodF
    self::__callStatic("a", "b", "c", "d");
    self::foo("a", "b", "c", "d");
  }
}
function main3() {
  $obj = new C3;
  $obj->test();
}
main3();

class A4 {
  public function foo($w, $x, $y, $z) {
    echo "A4::foo\n";
  }
}
class B4 extends A4 {
  public static function __callStatic($fn, $args) {
    var_dump(isset($this));
    var_dump($fn, $args);
    echo "\n";
  }
}
class C4 extends B4 {
}
function main4() {
  $fn = 'foo';
  C4::foo("a", "b", "c", "d");
  C4::$fn("a", "b", "c", "d");
}
main4();

class A5 {
  public static function __callStatic($fn, $args) {
    var_dump(isset($this));
    var_dump($fn, $args);
  }
}
class B5 extends A5 {
  public function test() {
    A5::foo("a", "b", "c", "d");
  }
}
class C5 extends B5 {
}

function main5() {
  $obj = new C5;
  $obj->test();
}
main5();

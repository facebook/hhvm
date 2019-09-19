<?hh

class C1 {
  public function __call($fn, $args) {
    var_dump($fn, $args);
    echo "\n";
  }
}

function main1() {
  $obj = new C1;

  // FCallObjMethodD
  $obj->__call("a", "b", "c", "d");
  $obj->foo("a", "b", "c", "d");

  // FCallObjMethod
  $fn = '__call';
  $obj->$fn("a", "b", "c", "d");
  $fn = 'foo';
  $obj->$fn("a", "b", "c", "d");
}

class C2 {
  public function __call($fn, $args) {
    echo "C2::__call\n";
    var_dump(isset($this));
    var_dump($fn, $args);
    echo "\n";
  }
  public function test() {
    // FCallClsMethodD
    C2::__call("a", "b", "c", "d");
    C2::foo("a", "b", "c", "d");

    // FCallClsMethod
    $cls = 'C2';
    $cls::__call("a", "b", "c", "d");
    $cls::foo("a", "b", "c", "d");
    $fn = '__call';
    C2::$fn("a", "b", "c", "d");
    $fn = 'foo';
    C2::$fn("a", "b", "c", "d");
    $fn = '__call';
    $cls::$fn("a", "b", "c", "d");
    $fn = 'foo';
    $cls::$fn("a", "b", "c", "d");

    // FCallClsMethodSD
    self::__call("a", "b", "c", "d");
    self::foo("a", "b", "c", "d");
  }
}

function main2() {
  $obj = new C2;
  $obj->test();
}

class B3 {
  public function __call($fn, $args) {
    var_dump($fn, $args);
    echo "\n";
  }
}
class C3 extends B3 {
}

function main3() {
  $obj = new C3;

  // FCallObjMethodD
  $obj->__call("a", "b", "c", "d");
  $obj->foo("a", "b", "c", "d");

  // FCallObjMethod
  $fn = '__call';
  $obj->$fn("a", "b", "c", "d");
  $fn = 'foo';
  $obj->$fn("a", "b", "c", "d");
}

class A4 {
  public function foo($w, $x, $y, $z) {
    echo "A4::foo\n";
  }
}
class B4 extends A4 {
  public function __call($fn, $args) {
    var_dump($fn, $args);
    echo "\n";
  }
}
class C4 extends B4 {
}

function main4() {
  $obj = new C4;
  $obj->foo("a", "b", "c", "d");
  $fn = 'foo';
  $obj->$fn("a", "b", "c", "d");
}

class A5 {
}
class B5 extends A5 {
  public function test() {
    A5::foo();
  }
}
class C5 extends B5 {
  public function __call($fn, $args) {
    var_dump($fn, $args);
  }
}

function main5() {
  $obj = new C5;
  $obj->test();
}

class A6 {
  public function __call($fn, $args) {
    var_dump($fn, $args);
  }
}
class B6 extends A6 {
  public function test() {
    A6::foo();
  }
}
class C6 extends B6 {
}

function main6() {
  $obj = new C6;
  $obj->test();
}
<<__EntryPoint>>
function main_entry(): void {

  main1();

  main2();

  main3();
  main4();

  main5();

  main6();
}

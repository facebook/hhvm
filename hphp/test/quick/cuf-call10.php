<?hh

class C1 {
  public function __call($fn, $args) {
    var_dump($fn, $args);
    echo "\n";
  }
}

function main1() {
  $obj = new C1;

  // call_user_func
  call_user_func(varray[$obj, "__call"], "a", "b", "c", "d");
  call_user_func(varray[$obj, "foo"], "a", "b", "c", "d");

  // call_user_func_array
  call_user_func_array(varray[$obj, "__call"], varray["a", "b", "c", "d"]);
  call_user_func_array(varray[$obj, "foo"], varray["a", "b", "c", "d"]);
}
main1();























class C3 {
  public function __call($fn, $args) {
    echo "C3::__call\n";
    var_dump(isset($this));
    var_dump($fn, $args);
    echo "\n";
  }

  public static function test() {
    // FCallClsMethodD
    call_user_func(varray['C3', 'foo'], "a", "b", "c", "d");
  }
}

function main3() {
  $obj = new C3;
  $obj::test();
}
main3();

class B4 {
  public function __call($fn, $args) {
    var_dump($fn, $args);
    echo "\n";
  }
}
class C4 extends B4 {
}

function main4() {
  $obj = new C4;
  call_user_func(varray[$obj, 'foo'], "a", "b", "c", "d");
}
main4();

class A5 {
  public function foo($w, $x, $y, $z) {
    echo "A5::foo\n";
  }
}
class B5 extends A5 {
  public function __call($fn, $args) {
    var_dump($fn, $args);
    echo "\n";
  }
}
class C5 extends B5 {
}

function main5() {
  $obj = new C5;
  call_user_func(varray[$obj, 'foo'], "a", "b", "c", "d");
}
main5();

class A6 {
}
class B6 extends A6 {
  public function test() {
    call_user_func('A6::foo', 1, 2, 3);
    call_user_func(varray['A6','foo'], 1, 2, 3);
  }
}
class C6 extends B6 {
  public function __call($fn, $args) {
    var_dump($fn, $args);
  }
}

function main6() {
  $obj = new C6;
  $obj->test();

  $obj = new B6;
  $obj->test();
}
main6();

class A7 {
  public function __call($fn, $args) {
    var_dump($fn, $args);
  }
}
class B7 extends A7 {
  public function test() {
    call_user_func(varray[$this, 'foo'], 1, 2, 3);
    call_user_func(varray[$this, 'foo'], 1, 2, 3);
  }
}
class C7 extends B7 {
}

function main7() {
  $obj = new C7;
  $obj->test();

  $obj = new B7;
  $obj->test();
}
main7();

class A8 {
  public function __call($fn, $args) {
    var_dump($fn, $args);
  }
}
class B8 {
  public function test() {
    call_user_func('A8::foo', 1, 2, 3);
    call_user_func(varray['A8', 'foo'], 1, 2, 3);
  }
}
class C8 extends B8 {
}

function main8() {
  $obj = new C8;
  $obj->test();
}
main8();

class C9 {
}
class D9 extends C9 {
  public function __call($fn, $args) {
    echo "D9::__call\n";
  }
}
class E9 extends D9 {
  public function __call($fn, $args) {
    echo "E9::__call\n";
  }
  public function test() {
    call_user_func(varray[$this, 'foo']);
    call_user_func(varray[new E9, 'foo']);
    call_user_func(varray[new E9, 'foo']);
  }
}
class F9 extends D9 {
  public function __call($fn, $args) {
    echo "F9::__call\n";
  }
}

function main9() {
  $obj = new E9;
  $obj->test();

  call_user_func(varray[$obj, 'foo']);
  call_user_func(varray['D9', 'foo']);
}
main9();

<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public function __construct() { echo "A::__construct\n"; }

  public function __call($a, $b) { echo "A::__call\n"; }

  public function __set($a, $b) { echo "A::__set\n"; }
  public function __get($a) { echo "A::__get\n"; return 123; }
  public function __isset($a) { echo "A::__isset\n"; return false; }
  public function __unset($a) { echo "A::__unset\n"; }

  public function __sleep() { echo "A::__sleep\n"; return varray[]; }
  public function __wakeup() { echo "A::__wakeup\n"; }

  public function __toString() { echo "A::__toString\n"; return ""; }

  public function __invoke() { echo "A::__invoke\n"; }

  public static function __set_state($a) { echo "A::__set_state\n"; return new stdclass; }
  public function __debugInfo() { echo "A::__debugInfo\n"; return varray[]; }

  public function __clone() { echo "A::__clone\n"; }
}

function test_magic_call() {
  echo "=============== test_magic_call ====================\n";
  $x = new A();
  $x->foo();

  $x->__call(1, 2);
}

function test_magic_props() {
  echo "=============== test_magic_props ===================\n";
  $x = new A();
  $x->foo = 123;
  $x->foo;
  isset($x->foo);
  unset($x->foo);

  $x->__set(1, 2);
  $x->__get(1);
  $x->__isset(1);
  $x->__unset(1);
}

function test_sleep() {
  echo "=============== test_sleep =========================\n";
  $x = new A();
  $serialized = serialize($x);
  unserialize($serialized);

  $x->__sleep();
  $x->__wakeup();
}

function test_stringify() {
  echo "=============== test_stringify =====================\n";
  $x = new A();
  (string)$x;

  $x->__toString();
}

function test_invoke() {
  echo "=============== test_invoke ========================\n";
  $x = new A();
  $x();

  $x->__invoke();

  array_map($x, varray[1]);
  Vector::fromItems(varray[true])->map($x);
  call_user_func($x, varray[]);
  call_user_func_array($x, varray[varray[]]);
}

function test_debug() {
  echo "=============== test_debug =========================\n";
  $x = new A();
  echo var_export($x, true) . "\n";
  var_dump($x);


  A::__set_state(1);
  $x->__debugInfo();
}

function test_clone() {
  echo "=============== test_clone =========================\n";
  $x = new A();
  clone $x;

  $x->__clone();
}

function test_ctor() {
  echo "=============== test_ctor =========================\n";
  $x = new A();

  $x->__construct();
}
<<__EntryPoint>> function main(): void {
test_magic_call();
test_magic_props();
test_sleep();
test_stringify();
test_invoke();
test_debug();
test_clone();
test_ctor();

echo "DONE\n";
}

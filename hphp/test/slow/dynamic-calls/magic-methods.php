<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public function __construct() { echo "A::__construct\n"; }

  public function __sleep() :mixed{ echo "A::__sleep\n"; return vec[]; }
  public function __wakeup() :mixed{ echo "A::__wakeup\n"; }

  public function __toString()[] :mixed{ echo "A::__toString\n"; return ""; }

  public function __invoke() :mixed{ echo "A::__invoke\n"; }

  public static function __set_state($a) :mixed{ echo "A::__set_state\n"; return new stdClass; }
  public function __debugInfo() :mixed{ echo "A::__debugInfo\n"; return vec[]; }

  public function __clone() :mixed{ echo "A::__clone\n"; }
}

function test_sleep() :mixed{
  echo "=============== test_sleep =========================\n";
  $x = new A();
  $serialized = serialize($x);
  unserialize($serialized);

  $x->__sleep();
  $x->__wakeup();
}

function test_stringify() :mixed{
  echo "=============== test_stringify =====================\n";
  $x = new A();
  (string)$x;

  $x->__toString();
}

function test_invoke() :mixed{
  echo "=============== test_invoke ========================\n";
  $x = new A();
  $x();

  $x->__invoke();

  array_map($x, vec[1]);
  Vector::fromItems(vec[true])->map($x);
  call_user_func($x, vec[]);
  call_user_func_array($x, vec[vec[]]);
}

function test_debug() :mixed{
  echo "=============== test_debug =========================\n";
  $x = new A();
  echo var_export($x, true) . "\n";
  var_dump($x);


  A::__set_state(1);
  $x->__debugInfo();
}

function test_clone() :mixed{
  echo "=============== test_clone =========================\n";
  $x = new A();
  clone $x;

  $x->__clone();
}

function test_ctor() :mixed{
  echo "=============== test_ctor =========================\n";
  $x = new A();

  $x->__construct();
}
<<__EntryPoint>> function main(): void {
test_sleep();
test_stringify();
test_invoke();
test_debug();
test_clone();
test_ctor();

echo "DONE\n";
}

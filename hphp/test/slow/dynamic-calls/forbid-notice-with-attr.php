<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

<<__DynamicallyCallable>>
function foo() {}

class A {
  <<__DynamicallyCallable>>
  public function foo() {}
  <<__DynamicallyCallable>>
  public static function bar() {}
}

function corge() {}

class A1 {
  public function corge() {}
  public static function flob() {}
}
<<__EntryPoint>>
function test() {
  echo "======== Called without using a function pointer ========\n\n";
  $foo = 'foo';
  $a_obj = new A();
  $a_bar = 'A::bar';
  $a_foo = varray[new A, 'foo'];
  $a = 'A';
  $bar = 'bar';

  $foo();
  $a_obj->$foo();
  $a_bar();
  $a_foo();

  echo "======== Negative tests (no notices) ========\n\n";

  HH\dynamic_fun($foo)();
  HH\dynamic_class_meth($a, $bar)();

  echo "======== Not __DynamicallyCallable ========\n\n";
  $corge = 'corge';
  $a1_obj = new A1();
  $a1_flob = 'A1::flob';
  $a1_corge = varray[new A1, 'corge'];
  $a1 = 'A1';
  $flob = 'flob';

  $corge();
  $a1_obj->$corge();
  $a1_flob();
  $a1_corge();

  echo "======== Not marked dynamic ========\n\n";

  HH\dynamic_fun($corge)();
  HH\dynamic_class_meth($a1, $flob)();
}

<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__DynamicallyCallable>>
function foo() :mixed{}

class A {
  <<__DynamicallyCallable>>
  public function foo() :mixed{}
  <<__DynamicallyCallable>>
  public static function bar() :mixed{}
}

function corge() :mixed{}

class A1 {
  public function corge() :mixed{}
  public static function flob() :mixed{}
}
<<__EntryPoint>>
function test() :mixed{
  echo "======== Called without using a function pointer ========\n\n";
  $foo = 'foo';
  $a_obj = new A();
  $a_bar = 'A::bar';
  $a_foo = vec[new A, 'foo'];
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
  $a1_corge = vec[new A1, 'corge'];
  $a1 = 'A1';
  $flob = 'flob';

  try { $corge(); } catch (Exception $e) { wrap($e); }
  try { $a1_obj->$corge(); } catch (Exception $e) { wrap($e); }
  try { $a1_flob(); } catch (Exception $e) { wrap($e); }
  try { $a1_corge(); } catch (Exception $e) { wrap($e); }

  echo "======== Not marked dynamic ========\n\n";

  HH\dynamic_fun($corge)();
  HH\dynamic_class_meth($a1, $flob)();
}

function wrap($e) :mixed{ echo "Exception: {$e->getMessage()}\n"; }

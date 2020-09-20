<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function wrap($e) { echo "Exception: {$e->getMessage()}\n"; }

function func1() {}
<<__DynamicallyCallable>> function func4() {}

class A {
  public function func2() {}
  public static function func3() {}
}

class B {
  <<__DynamicallyCallable>> public function func5() {}
  <<__DynamicallyCallable>> public static function func6() {}
}

function positive_tests() {
  echo "=========================== positive tests ===================\n";
  try { (new ReflectionFunction('func1'))->invoke(); } catch (Exception $e) { wrap($e); }
  try { (new ReflectionMethod('A::func2'))->invoke(new A); } catch (Exception $e) { wrap($e); }
  try { (new ReflectionMethod('A::func3'))->invoke(new A); } catch (Exception $e) { wrap($e); }
}

function negative_tests() {
  echo "=========================== negative tests ===================\n";
  (new ReflectionFunction('func4'))->invoke();
  (new ReflectionMethod('B::func5'))->invoke(new B);
  (new ReflectionMethod('B::func6'))->invoke(new B);
}
<<__EntryPoint>> function main(): void {
positive_tests();
negative_tests();
}

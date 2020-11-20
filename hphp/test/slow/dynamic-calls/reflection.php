<?hh
// Copyright 2004-present Facebook. All Rights Reserved.



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
  (new ReflectionFunction('func1'))->invoke();
  (new ReflectionMethod('A::func2'))->invoke(new A);
  (new ReflectionMethod('A::func3'))->invoke(new A);
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

<?hh
// Copyright 2004-present Facebook. All Rights Reserved.



function func1() :mixed{}
<<__DynamicallyCallable>> function func4() :mixed{}

class A {
  public function func2() :mixed{}
  public static function func3() :mixed{}
}

class B {
  <<__DynamicallyCallable>> public function func5() :mixed{}
  <<__DynamicallyCallable>> public static function func6() :mixed{}
}

function positive_tests() :mixed{
  echo "=========================== positive tests ===================\n";
  (new ReflectionFunction('func1'))->invoke();
  (new ReflectionMethod('A::func2'))->invoke(new A);
  (new ReflectionMethod('A::func3'))->invoke(new A);
}

function negative_tests() :mixed{
  echo "=========================== negative tests ===================\n";
  (new ReflectionFunction('func4'))->invoke();
  (new ReflectionMethod('B::func5'))->invoke(new B);
  (new ReflectionMethod('B::func6'))->invoke(new B);
}
<<__EntryPoint>> function main(): void {
positive_tests();
negative_tests();
}

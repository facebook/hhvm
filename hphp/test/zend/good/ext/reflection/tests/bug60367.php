<?hh

abstract class A {
  const WHAT = 'A';

  <<__DynamicallyCallable>> public static function call() :mixed{
    echo static::WHAT;
  }
}

class B extends A {
  const WHAT = 'B';
}

<<__EntryPoint>> function main(): void {
  $method = new ReflectionMethod("B::call");
  $method->invoke(null);
  $method->invokeArgs(null, vec[]);
  $method = new ReflectionMethod("A::call");
  $method->invoke(null);
  $method->invokeArgs(null, vec[]);
}

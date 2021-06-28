<?hh

abstract class A {
  const WHAT = 'A';

  <<__DynamicallyCallable>> public static function call() {
    echo static::WHAT;
  }
}

class B extends A {
  const WHAT = 'B';
}

<<__EntryPoint>> function main(): void {
  $method = new ReflectionMethod("B::call");
  $method->invoke(null);
  $method->invokeArgs(null, varray[]);
  $method = new ReflectionMethod("A::call");
  $method->invoke(null);
  $method->invokeArgs(null, varray[]);
}

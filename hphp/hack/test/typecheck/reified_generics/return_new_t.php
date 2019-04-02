<?hh

final class A {}

function f<<<__Newable>> reify T as A>(): T {
  $a = new T();
  return $a;
}

function g<<<__Newable>> reify T as A>(): T {
  return new T();
}

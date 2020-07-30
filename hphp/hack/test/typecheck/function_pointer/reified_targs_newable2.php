<?hh

final class A<reify T> {}

function foo<<<__Newable>> reify T as A<string>>(): dynamic {
  return new T();
}

function test(): void {
  $x = foo<A<string>>;

  $y = foo<A<int>>;
}

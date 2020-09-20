<?hh

type t = <<__Soft>> darray<int, <<__Soft>> string>;
type u = varray<<<__Soft>> int>;

function f<T>(): void {}

function g(<<__Soft>> int $_): <<__Soft>> string {
  f<<<__Soft>> float>();
  return "hello";
}

abstract class C {
  <<__Soft>> protected float $x;
  const type T = <<__Soft>> int;

  public async function f(): Awaitable<<<__Soft>> int> {
    return 42;
  }
}

class D {
  const <<__Soft>> int X = 0;
}

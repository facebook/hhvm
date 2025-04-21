<?hh

abstract class C {
  abstract const type T;
}
class D extends C {
  const type T = int;
}

abstract class X {
  <<__Reifiable>>
  abstract const type T as C with { type T = int };

  public function test(): void {
    f<this::T>();
  }
}
class Y extends X {
  const type T = D;
}
class Z extends X {
  // illegal by NastCheck as well as by reified check
  const type T = C with { type T = int };
}

function f<reify T>(): void {}

function test(): void {
  // illegal by reified check
  f<C with { type T = int }>();
}

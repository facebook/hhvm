<?hh

class A {}

abstract class B {}
<<__ConsistentConstruct>> class C extends B {}

<<__ConsistentConstruct>> abstract class D {}
class E extends D {}

function ftest<
  <<__Newable>> reify Tf as D
>(): void {}

class Test<
  <<__Newable>> reify Tc as A as B as C as D as E,
> {
  public function f<
    <<__Newable>> reify Tm
  >(Tc $s): void {}
}

<?hh

<<__Pure>>
function pure(Pure<(function(): int)> $a): int {
  return $a();
}

<<__Pure>>
function returnspure(): Pure<(function(): int)> {
  return () ==> 1;
}

class A {
  <<__Pure>>
  public function f(): void {
  }
}

<<__Rx>>
function f(
  Pure<(function (Mutable<A>): void)> $f1,
  Pure<(function (MaybeMutable<A>): void)> $f2,
  Pure<(function (OwnedMutable<A>): void)> $f3): void {
  $a = \HH\Rx\mutable(new A());
  $f1($a);
  $f2($a);
  $f3(\HH\Rx\move($a));
}

<<__RxLocal, __EntryPoint>>
function g(): void {
  pure(<<__Pure>>() ==> 1);
  pure(returnspure());

  f(<<__Pure>>(A $a) ==> {},
    <<__Pure>>(A $a) ==> {},
    <<__Pure>>(A $a) ==> {});
}

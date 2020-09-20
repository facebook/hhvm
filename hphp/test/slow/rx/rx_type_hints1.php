<?hh

<<__Rx>>
function rx(Rx<(function(): int)> $a): int {
  return $a();
}

<<__RxLocal>>
function local(RxLocal<(function(): int)> $a): int {
  return $a();
}

<<__RxShallow>>
function shallow(RxShallow<(function(): int)> $a): int {
  return $a();
}

<<__Rx>>
function returnsrx(): Rx<(function(): int)> {
  return () ==> 1;
}

class A {
  <<__Rx, __Mutable>>
  public function f(): void {
  }
}

<<__Rx>>
function f(
  Rx<(function (Mutable<A>): void)> $f1,
  Rx<(function (MaybeMutable<A>): void)> $f2,
  Rx<(function (OwnedMutable<A>): void)> $f3): void {
  $a = \HH\Rx\mutable(new A());
  $f1($a);
  $f2($a);
  $f3(\HH\Rx\move($a));
}

<<__RxLocal, __EntryPoint>>
function g(): void {
  rx(<<__Rx>>() ==> 1);
  local(<<__RxLocal>>() ==> 1);
  shallow(<<__RxShallow>>() ==> 1);
  local(() ==> 1);
  rx(returnsrx());

  f(<<__Rx>>(<<__Mutable>> A $a) ==> {},
    <<__Rx>>(<<__MaybeMutable>> A $a) ==> {},
    <<__Rx>>(<<__OwnedMutable>> A $a) ==> {});
}

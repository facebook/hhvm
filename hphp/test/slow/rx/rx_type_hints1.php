<?hh

function rx(Rx<(function(): int)> $a)[rx]: int {
  return $a();
}

function local(RxLocal<(function(): int)> $a)[rx_local]: int {
  return $a();
}

function shallow(RxShallow<(function(): int)> $a)[rx_shallow]: int {
  return $a();
}

function returnsrx()[rx]: Rx<(function(): int)> {
  return () ==> 1;
}

class A {
  public function f()[rx]: void {
  }
}

function f(
  Rx<(function (Mutable<A>): void)> $f1,
  Rx<(function (MaybeMutable<A>): void)> $f2,
  Rx<(function (OwnedMutable<A>): void)> $f3)[rx]: void {
  $a = \HH\Rx\mutable(new A());
  $f1($a);
  $f2($a);
  $f3(\HH\Rx\move($a));
}

<<__RxLocal, __EntryPoint>>
function g()[rx_local]: void {
  rx(<<__Rx>>() ==> 1);
  local(<<__RxLocal>>() ==> 1);
  shallow(<<__RxShallow>>() ==> 1);
  local(() ==> 1);
  rx(returnsrx());

  f(<<__Rx>>(A $a) ==> {},
    <<__Rx>>(A $a) ==> {},
    <<__Rx>>(A $a) ==> {});
}

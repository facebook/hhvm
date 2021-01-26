<?hh

function pure(Pure<(function(): int)> $a)[]: int {
  return $a();
}

function returnspure()[]: Pure<(function(): int)> {
  return () ==> 1;
}

class A {
  public function f()[]: void {
  }
}

function f(
  Pure<(function (Mutable<A>): void)> $f1,
  Pure<(function (MaybeMutable<A>): void)> $f2,
  Pure<(function (OwnedMutable<A>): void)> $f3)[rx]: void {
  $a = \HH\Rx\mutable(new A());
  $f1($a);
  $f2($a);
  $f3(\HH\Rx\move($a));
}

<<__RxLocal, __EntryPoint>>
function g()[rx_local]: void {
  pure(()[] ==> 1);
  pure(returnspure());

  f((A $a)[] ==> {},
    (A $a)[] ==> {},
    (A $a)[] ==> {});
}

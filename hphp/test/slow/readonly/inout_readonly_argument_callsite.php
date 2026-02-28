<?hh

class Foo {}
function inout_fun(inout vec<Foo> $y): void {}


function foo(readonly Foo $x, readonly Foo $y): void {
  $a = vec[$x, $y]; // $y is inout
  inout_fun(inout $a);
}

<<__EntryPoint>>
function test(): void {
  foo(new Foo(), new Foo());
}

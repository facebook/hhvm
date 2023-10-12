<?hh // strict

class C<+T> { }
class A { }
class B { }
class E extends C<B> { }

function test_flow<T>(C<T> $c, bool $flag):T {
  if ($flag) {
    echo "do nothing";
  } else {
    invariant($c is E, "e");
  }
  return new B();
}

function expect_A(A $a):void { }

<<__EntryPoint>>
function breakit():void {
  $a = test_flow(new C<A>(), true);
  expect_A($a);
}

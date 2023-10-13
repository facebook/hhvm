<?hh

class C<+T> { }
class A { }
class B { }
class E extends C<B> { }

function test_flow<T>(vec<C<T>> $cs, bool $flag):T {
  foreach ($cs as $c) {
    if (!$c is E) break;
  }
  return new B();
}

function expect_A(A $a):void { }

<<__EntryPoint>>
function breakit():void {
  $a = test_flow(vec[new C<A>()], true);
  expect_A($a);
}

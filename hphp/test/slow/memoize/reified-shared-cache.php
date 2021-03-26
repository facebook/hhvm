<?hh

class C {
  // We need to use more than 10 memoized functions, to trigger the
  // "shared memo cache" sparse representation. If we do that, the
  // "number of params" accounting for the reified function needs to be
  // correct.

  <<__Memoize>> function f<reify T>() { echo "in f\n"; }

  <<__Memoize>> function f0() { echo "in f0\n"; }
  <<__Memoize>> function f1() { echo "in f1\n"; }
  <<__Memoize>> function f2() { echo "in f2\n"; }
  <<__Memoize>> function f3() { echo "in f3\n"; }
  <<__Memoize>> function f4() { echo "in f4\n"; }
  <<__Memoize>> function f5() { echo "in f5\n"; }
  <<__Memoize>> function f6() { echo "in f6\n"; }
  <<__Memoize>> function f7() { echo "in f7\n"; }
  <<__Memoize>> function f8() { echo "in f8\n"; }
  <<__Memoize>> function f9() { echo "in f9\n"; }
}

<<__EntryPoint>>
function main() {
  $c = new C();
  $c->f<C>();
  $c->f0();
}

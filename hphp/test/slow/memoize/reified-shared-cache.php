<?hh

class C {
  // We need to use more than 10 memoized functions, to trigger the
  // "shared memo cache" sparse representation. If we do that, the
  // "number of params" accounting for the reified function needs to be
  // correct.

  <<__Memoize>> function f<reify T>() :mixed{ echo "in f\n"; }

  <<__Memoize>> function f0() :mixed{ echo "in f0\n"; }
  <<__Memoize>> function f1() :mixed{ echo "in f1\n"; }
  <<__Memoize>> function f2() :mixed{ echo "in f2\n"; }
  <<__Memoize>> function f3() :mixed{ echo "in f3\n"; }
  <<__Memoize>> function f4() :mixed{ echo "in f4\n"; }
  <<__Memoize>> function f5() :mixed{ echo "in f5\n"; }
  <<__Memoize>> function f6() :mixed{ echo "in f6\n"; }
  <<__Memoize>> function f7() :mixed{ echo "in f7\n"; }
  <<__Memoize>> function f8() :mixed{ echo "in f8\n"; }
  <<__Memoize>> function f9() :mixed{ echo "in f9\n"; }
}

<<__EntryPoint>>
function main() :mixed{
  $c = new C();
  $c->f<C>();
  $c->f0();
}

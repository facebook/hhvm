<?hh

class C {
<<__Memoize(#KeyedByIC)>>
function test()[] {
  echo "ok\n";
}
}

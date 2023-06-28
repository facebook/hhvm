<?hh

class C {
<<__Memoize(#KeyedByIC)>>
function test()[] :mixed{
  echo "ok\n";
}
}

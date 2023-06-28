<?hh

<<__Memoize(#KeyedByIC)>>
function f()[zoned] :mixed{
  echo "ok\n";
}

<<__EntryPoint>>
function main() :mixed{
  f(); f(); f();
}

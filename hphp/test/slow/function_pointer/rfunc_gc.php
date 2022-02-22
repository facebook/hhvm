<?hh

function baz<reify T1, reify T2>() {
  return 42;
}

function foo<reify T>() {
  $func = baz<T, int>;
  gc_collect_cycles();
  gc_collect_cycles();
  return $func();
}

<<__EntryPoint>>
function main() {
  var_dump(foo<string>());
}

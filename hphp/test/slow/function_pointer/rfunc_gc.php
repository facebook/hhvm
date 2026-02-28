<?hh

function baz<reify T1, reify T2>() :mixed{
  return 42;
}

function foo<reify T>() :mixed{
  $func = baz<T, int>;
  gc_collect_cycles();
  gc_collect_cycles();
  return $func();
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(foo<string>());
}

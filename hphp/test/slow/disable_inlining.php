<?hh

<<__NEVER_INLINE>>
function foo() :mixed{
  return 4;
}

<<__NEVER_INLINE>>
function bar($x) :mixed{
  echo var_dump($x[3]);
}

function main() :mixed{
  $a = vec[1, 2, 3, foo()];
  bar($a);
}


<<__EntryPoint>>
function main_disable_inlining() :mixed{
main();
}

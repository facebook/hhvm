<?hh

function foo($p) :mixed{
  return $p;
}


<<__EntryPoint>>
function main_eventually_unfoldable_across_blocks() :mixed{
$a = 42;
var_dump(foo(true ? 0 : 1));
foo($a);
}

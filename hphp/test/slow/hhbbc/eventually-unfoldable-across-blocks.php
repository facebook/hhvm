<?hh

function foo($p) {
  return $p;
}


<<__EntryPoint>>
function main_eventually_unfoldable_across_blocks() {
$a = 42;
var_dump(foo(true ? 0 : 1));
foo($a);
}

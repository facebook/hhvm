<?hh

function bar($x) {
  // just here to make sure foo isn't a leaf function
  // since FCallBuiltin doesn't count as a php-call.
}

function foo($x) {
  $x = (array)$x;
  array_map(fun("foo"), $x);
  bar($x);
}


<<__EntryPoint>>
function main_reenter() {
foo(1);
}

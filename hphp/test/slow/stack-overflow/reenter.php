<?hh

function bar($x) :mixed{
  // just here to make sure foo isn't a leaf function
  // since FCallBuiltin doesn't count as a php-call.
}

function foo($x) :mixed{
  $x = vec[$x];
  array_map(foo<>, $x);
  bar($x);
}


<<__EntryPoint>>
function main_reenter() :mixed{
foo(1);
}

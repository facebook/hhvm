<?php /* destructor */

function foo() {
 var_dump('failed');
 return 12;
}

class lol { public function __destruct() { echo "lol\n"; } }
function & bar() {
  var_dump(func_get_args());
  $x = new lol();
  return $x;
}

function main() {
  var_dump(foo(1,2));
  $l = foo(1,2);
}



<<__EntryPoint>>
function main_extra_args() {
fb_intercept('foo', 'bar', "hello");

main();
echo "done\n";
}

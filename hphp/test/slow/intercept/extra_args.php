<?hh

function foo() {
 var_dump('failed');
 return 12;
}

fb_intercept('foo', 'bar', "hello");

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

main();
echo "done\n";


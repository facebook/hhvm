<?hh /* destructor */

function foo(...$args) {
 var_dump('failed');
 return 12;
}

class lol {}
function bar($_1, $_2, inout $_3, $_4, inout $_5) {
  var_dump(varray[$_1, $_2, $_3, $_4, $_5]);
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

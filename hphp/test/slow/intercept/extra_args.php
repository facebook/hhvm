<?hh /* destructor */

function foo(...$args) {
 var_dump('failed');
 return 12;
}

class lol {}
function bar($_1, $_2, inout $_3) {
  var_dump(varray[$_1, $_2, $_3]);
  $x = new lol();
  return shape('value' => $x);
}

function main() {
  var_dump(foo(1,2));
  $l = foo(1,2);
}



<<__EntryPoint>>
function main_extra_args() {
fb_intercept2('foo', 'bar');

main();
echo "done\n";
}

<?hh /* destructor */

function foo(...$args) :mixed{
 var_dump('failed');
 return 12;
}

class lol {}

<<__DynamicallyCallable>>
function bar($_1, $_2, inout $_3) :mixed{
  var_dump(vec[$_1, $_2, $_3]);
  $x = new lol();
  return shape('value' => $x);
}

function main() :mixed{
  var_dump(foo(1,2));
  $l = foo(1,2);
}



<<__EntryPoint>>
function main_extra_args() :mixed{
fb_intercept2('foo', 'bar');

main();
echo "done\n";
}

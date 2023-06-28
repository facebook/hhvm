<?hh

function foo(inout $a) :mixed{
  var_dump('foo');
  $a[0] = 1;
}
function bar(inout $a) :mixed{
  var_dump('bar');
  $a[0] = 2;
}
function goo($name, $obj, inout $params) :mixed{
  return shape('value' => bar(inout $params));
}

<<__EntryPoint>>
function main_1198() :mixed{
fb_intercept2('foo', 'goo');
$a = 0;
foo(inout $a);
var_dump($a);
}

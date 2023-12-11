<?hh
<<__DynamicallyCallable>>
function foo(inout $x) :mixed{
  $x = 42;
}
<<__DynamicallyCallable>>
function bar(inout $a, inout $b) :mixed{
  list($a, $b) = vec[$b, $a];
  return HH\Lib\Legacy_FIXME\cast_for_arithmetic($a) + HH\Lib\Legacy_FIXME\cast_for_arithmetic($b);
}
<<__DynamicallyCallable>>
function baz(inout $q) :mixed{
  $q = debug_backtrace()[0]['function'];
  return 12;
}

function main($a, $b, $c) :mixed{
  $foo = 0;
  $bar1 = 'a';
  $bar2 = 'b';
  $baz = null;
  $a(inout $foo);
  $b(inout $bar1, inout $bar2);
  $c(inout $baz);

  var_dump($foo, $bar1, $bar2, $baz);
}


<<__EntryPoint>>
function main_call_dynamic() :mixed{
if (!isset($x)) $x = 'foo';
if (!isset($y)) $y = 'bar';
if (!isset($z)) $z = 'baz';
main($x, $y, $z);
}

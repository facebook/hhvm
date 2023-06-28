<?hh

function foo() :mixed{
  return '1';
}
function bar() :mixed{
  $a = 1;
  $a += HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a);
  $b = 1;
  $b -= HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($b);
}

<<__EntryPoint>>
function main_1434() :mixed{
bar();
}

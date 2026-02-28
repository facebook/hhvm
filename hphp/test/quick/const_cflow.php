<?hh

function f(inout $x) :mixed{ var_dump($x); }
function test($b, $c) :mixed{
  $x = false && $b;
  $x = HH\Lib\Legacy_FIXME\cast_for_arithmetic($x);
  $x += HH\Lib\Legacy_FIXME\cast_for_arithmetic(true && $b);
  $x += HH\Lib\Legacy_FIXME\cast_for_arithmetic(false || $b);
  $x += HH\Lib\Legacy_FIXME\cast_for_arithmetic(true || $b);

  $x += false ? $b : $c;
  $x += true ? $b : $c;
  f(inout $x);
}
<<__EntryPoint>> function main(): void {
test(2, 3);
}

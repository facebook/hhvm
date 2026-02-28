<?hh

function foo($a, inout $b, inout $c, $d) :mixed{
  $a = 10;
  $b = 20;
  $c = HH\Lib\Legacy_FIXME\cast_for_arithmetic($c);
  $c *= 10;
  $d *= 10;
  echo (__METHOD__."(): a: ".$a.", b: ".$b.", c: ".$c.", d: ".$d."\n");
}

<<__EntryPoint>> function main(): void {
  $a = 1;
  $b = 2;
  $c = 3;
  $d = 4;
  echo (__METHOD__."(): a: ".$a.", b: ".$b.", c: ".$c.", d: ".$d."\n");
  foo($a, inout $b, inout $c, $d);
  echo (__METHOD__."(): a: ".$a.", b: ".$b.", c: ".$c.", d: ".$d."\n");

  $foo = null;
  $bar = null;
  foo(123, inout $foo, inout $bar, 456);  // should not warn that $foo/$bar are undefined
  var_dump($foo, $bar);
}

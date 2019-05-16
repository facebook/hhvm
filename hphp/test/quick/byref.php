<?hh

function foo($a, &$b, &$c, $d) {
  $a = 10;
  $b = 20;
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
  foo($a, &$b, &$c, $d);
  echo (__METHOD__."(): a: ".$a.", b: ".$b.", c: ".$c.", d: ".$d."\n");

  foo(123, &$foo, &$bar, 456);  // should not warn that $foo/$bar are undefined
  var_dump($foo, $bar);
}

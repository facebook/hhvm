<?php

function foo($a, &$b, &$c, $d) {
  $a = 10;
  $b = 20;
  $c *= 10;
  $d *= 10;
  echo (__METHOD__."(): a: ".$a.", b: ".$b.", c: ".$c.", d: ".$d."\n");
}

function main() {
  $a = 1;
  $b = 2;
  $c = 3;
  $d = 4;
  echo (__METHOD__."(): a: ".$a.", b: ".$b.", c: ".$c.", d: ".$d."\n");
  foo($a, $b, $c, $d);
  echo (__METHOD__."(): a: ".$a.", b: ".$b.", c: ".$c.", d: ".$d."\n");

  sscanf("123", "%d", $number);  // should not warn that $number is undefined
  var_dump($number);
}

main();

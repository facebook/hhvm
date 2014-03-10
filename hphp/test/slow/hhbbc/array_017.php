<?php

function four() { return 4; }
function heh2() { return array('heh' => four()); }
function heh() { return array('foo' => heh2()); }
function bar() { return array('other' => heh()); }
function foo() {
  $x = bar();
  $x['other']['foo']['heh'] = 2;
  $x['other']['whatever'] = 2;
  $x['yoyo'] = array('stuff' => $x);
  return $x;
}
function main() {
  $x = foo();
  echo $x['other']['foo']['heh'] . "\n";
  echo $x['other']['whatever'] . "\n";
  var_dump($x['yoyo']['stuff']['other']['foo']['heh']);
  var_dump($x['yoyo']['stuff']);
}
main();

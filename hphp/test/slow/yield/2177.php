<?php

function makeClosureCont() {
  return function () {
    static $x = 0;
    yield $x++;
    yield $x++;
  }
;
}
function gen() {
  static $x = 0;
  yield $x++;
  yield $x++;
}
$cc = makeClosureCont();
foreach ($cc() as $v) {
 var_dump($v);
 }
$cc1 = makeClosureCont();
foreach ($cc1() as $v) {
 var_dump($v);
 }
foreach (gen() as $v) {
 var_dump($v);
 }
foreach (gen() as $v) {
 var_dump($v);
 }

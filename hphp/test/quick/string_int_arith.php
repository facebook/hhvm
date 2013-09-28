<?php

function zero() { return 0; }
function foo() { return "0x10"; }
function twelve() { return 12; }

function main() {
  var_dump(zero() + foo());
  var_dump(zero() - foo());
  var_dump(zero() / foo());
  var_dump(zero() * foo());

  var_dump(foo() + zero());
  var_dump(foo() - zero());
  var_dump(foo() / zero());
  var_dump(foo() * zero());

  var_dump(twelve() + foo());
  var_dump(twelve() - foo());
  var_dump(twelve() / foo());
  var_dump(twelve() * foo());

  var_dump(foo() + twelve());
  var_dump(foo() - twelve());
  var_dump(foo() / twelve());
  var_dump(foo() * twelve());
}

function setop_main() {
  $a = array(zero());
  $a[0] += foo();
  var_dump($a[0]);
  $a = array(zero());
  $a[0] -= foo();
  var_dump($a[0]);
  $a = array(zero());
  $a[0] /= foo();
  var_dump($a[0]);
  $a = array(zero());
  $a[0] *= foo();
  var_dump($a[0]);

  $a = array(foo());
  $a[0] += zero();
  var_dump($a[0]);
  $a = array(foo());
  $a[0] -= zero();
  var_dump($a[0]);
  $a = array(foo());
  $a[0] /= zero();
  var_dump($a[0]);
  $a = array(foo());
  $a[0] *= zero();
  var_dump($a[0]);

  $a = array(twelve());
  $a[0] += foo();
  var_dump($a[0]);
  $a = array(twelve());
  $a[0] -= foo();
  var_dump($a[0]);
  $a = array(twelve());
  $a[0] /= foo();
  var_dump($a[0]);
  $a = array(twelve());
  $a[0] *= foo();
  var_dump($a[0]);

  $a = array(foo());
  $a[0] += twelve();
  var_dump($a[0]);
  $a = array(foo());
  $a[0] -= twelve();
  var_dump($a[0]);
  $a = array(foo());
  $a[0] /= twelve();
  var_dump($a[0]);
  $a = array(foo());
  $a[0] *= twelve();
  var_dump($a[0]);
}

main();
setop_main();


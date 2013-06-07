<?php
// Copyright 2004-present Facebook. All Rights Reserved.

// Test flow control around exceptions. Stepping into/over/out of throws,
// catches, stepping over calls that have throw/catch within, etc.
class Ex1 extends Exception {
}

class Ex2 extends Exception {
}

function throwAndCatch($a) {
  $x = $a + 1;
  try {
    throw new Ex2('Thrown from throwAndCatch '.$x);
  } catch (Ex2 $e) {
    printf("Caught %s in throwAndCatch()\n", $e->getMessage());
    $x++;
  }
  return $x;
}

function throwNoCatch($a) {
  $x = $a + 1;
  $e = new Ex1('Thrown from throwNoCatch '.$x);
  throw $e;
}

function throwFromCallee($a) {
  $x = $a + 1;
  try {
    throwNoCatch($x);
  } catch (Ex1 $e) {
    printf("Caught %s in throwFromCallee()\n", $e->getMessage());
    $x++;
  }
  return $x;
}

function foo($a) {
  $x = $a + 1;
  $x = throwAndCatch($x);
  try {
    $x = throwAndCatch($x);
    throw new Ex1('Thrown from foo '.$x);
  } catch (Exception $e) {
    printf("Caught %s in foo()\n", $e->getMessage());
    $x = throwAndCatch($x);
    $x++;
  }
  return $x;
}

function main() {
  $x = throwFromCallee(1);
  var_dump($x);

  $x = foo(2);
  var_dump($x);
}

main();



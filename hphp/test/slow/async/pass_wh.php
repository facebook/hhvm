<?php

async function bar() {
  return 1;
}

async function foo() {
  $b = await bar();
  return 1 + $b;
}

function giveMeWaitHandle() {
  return foo();
}

function giveMeValue() {
  $wh = giveMeWaitHandle();
  return $wh->join();
}

var_dump(giveMeValue());


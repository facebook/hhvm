<?php

error_reporting(-1);

function heh() {
  static $x = 0;
  echo "ok $x\n";
  if ($x++ == 0) {
    throw new Exception('a');
  }
}
set_error_handler('heh');

function foo() {
  try {
    $x++;
  } catch (Exception $y) {
    // $x: Uninit here.  This is a divergence from php5 (where it will
    // be int(1)).
    var_dump($x);
  }
}

foo();

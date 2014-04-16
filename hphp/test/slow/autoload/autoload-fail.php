<?php

// Copyright 2004-2014 Facebook. All Rights Reserved.

function fail($t, $n, $e) {
  var_dump($t, $n);
  if ($e instanceof Exception) {
    var_dump($e->getMessage(), $e->getTrace());
  } else {
    var_dump($e);
  }
  if ($n == 'C') {
    class C {}
  } else if ($n == 'D') {
    class D {}
  }
}

HH\autoload_set_paths(array('class' => array('c' => 'autoload-fail-c.inc',
                                             'd' => 'autoload-fail-d.inc'),
                            'failure' => 'fail'), __DIR__ . '/');

$x = new C;
$x = new D;

<?php

function fail($t, $n, $e) {
  var_dump($t, $n);
  if ($e instanceof Exception) {
    var_dump($e->getMessage(), $e->getTrace());
  } else {
    var_dump($e);
  }
  return false; // stop: otherwise hoistability would create an infinite loop
}

\HH\autoload_set_paths(
  array('class' => array(//
          'i1' => 'autoload-fail-e.inc',
          'i2' => 'autoload-fail-e.inc',
        ),
        'failure' => 'fail',
       ),
  __DIR__ . '/'
);

class X implements I1 { public function method() { echo __METHOD__,"\n"; }}
$x = new X();
$x->method();
echo 'NOTE: repo-mode doesn\'t invoke the autoloader at all (I1 is in repo)', "\n";
echo 'Done', "\n";

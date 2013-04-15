<?php

function a(callable $b) { $b(); }

$c = function() { var_dump(true); };
a($c);

try {
  a('hi');
} catch (Exception $e) {
  var_dump($e);
}

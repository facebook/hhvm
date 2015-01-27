<?php

function foo() { }

$data = array(
  'foo',
  'strtolower',
  1,
  1.1231,
  function ($str, $start, $end) { return array(); },
);

foreach ($data as $callback) {
  readline_completion_function($callback);
}

<?php

function foo() { }


<<__EntryPoint>>
function main_readline_completion_function() {
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
}

<?php

function foo(array $test) {
  foreach ($test AS $var) {
    global $$var;
    $$var = $var . 'foo';
  }
}
foo(array('a', 'b'));
var_dump($a, $b);

<?php

function foo($x) {
  set_time_limit(1);
  foreach ($x as &$v) {
    $x = array(0);
  }
}
foo(array(1,2,3,4));

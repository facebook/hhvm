<?php

$keys = array(null, true, false, 0, 100, 0.0, 1238.93498);

foreach ($keys as $key) {
  $a = array();
  $a[$key] = 123;
  var_dump($a);
  var_dump($a[$key]);
}

<?php

function a() {
  $x[1e26] = 2;
  var_dump($x);
  foreach ($x as $k => $v) { var_dump(is_int($k), is_int($v)); }
}
a();

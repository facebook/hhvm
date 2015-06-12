<?php

function x() {
  $y1 = new stdclass;
  $y = new stdclass;
  $y->dyn = $y1;
  unset($y1);
  var_dump($y);
  unset($y);
  var_dump(new stdclass);
}
x();

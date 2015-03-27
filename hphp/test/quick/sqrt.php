<?php

function neg_zero() {
  $zero = -1.0;
  while ($zero != 0.0) {
    $zero /= 2.0;
  }
  return $zero;
}

function main($f) {
  var_dump(sqrt(0.0));
  var_dump(sqrt(neg_zero()));
  var_dump(sqrt(5));
  var_dump(sqrt(2.5));
  var_dump(sqrt(-3));
  var_dump(sqrt($f));
  var_dump(sqrt(true));
  var_dump(sqrt(false));
  var_dump(sqrt(null));
  var_dump(sqrt("15"));
  var_dump(sqrt("hello"));
  var_dump(sqrt(new stdClass));
  var_dump(sqrt(array()));
  var_dump(sqrt(array(2,3,4)));
}
main(2.241987);

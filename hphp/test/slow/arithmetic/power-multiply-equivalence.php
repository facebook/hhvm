<?php

  <<__EntryPoint>>
function main_power_multiply_equivalence() {
var_dump(3 ** 2);
  var_dump(3 ** 2.0);
  var_dump(3 ** 4);
  var_dump(PHP_INT_MAX ** 2);
  var_dump(PHP_INT_MIN ** 2);
  var_dump(acos(8) ** 2);
  var_dump(0 ** 2);

  var_dump(2 ** 3);
  var_dump(2 ** 3.0);
  var_dump(PHP_INT_MAX ** 3);
  var_dump(PHP_INT_MIN ** 3);
  var_dump(acos(8) ** 3);
  var_dump(0 ** 3);
}

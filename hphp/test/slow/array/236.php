<?php

function foo() {
  $a = array();
  $a[] = '1.1';
  $a[] = '2.2';
  $a[] = '3.3';
  var_dump(array_sum($a));
  var_dump(array_product($a));
}
foo();

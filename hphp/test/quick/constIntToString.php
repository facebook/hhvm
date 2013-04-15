<?php

function foo() {
  $x = 123;
  var_dump((string)$x);
  $y = -456;
  var_dump((string)$y);
  $z = 123456789123456789123456789;
  var_dump((string)$z);
} 

foo();


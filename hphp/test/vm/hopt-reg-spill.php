<?php
function foo($t0, $t1, $t2, $t3, $t4, $t5, $t6, $t7,
             $t8, $t9, $t10, $t11, $t12, $t13, $t14, $t15) {
  $sum = 0;

  $sum = $sum + $t0;
  $sum = $sum + $t1;
  $sum = $sum + $t2;
  $sum = $sum + $t3;
  $sum = $sum + $t4;
  $sum = $sum + $t5;
  $sum = $sum + $t6;
  $sum = $sum + $t7;
  $sum = $sum + $t8;
  $sum = $sum + $t9;
  $sum = $sum + $t10;
  $sum = $sum + $t11;
  $sum = $sum + $t12;
  $sum = $sum + $t13;
  $sum = $sum + $t14;
  $sum = $sum + $t15;

  $tmp = $t0; $t0 = $t15; $t15 = $tmp;
  $tmp = $t1; $t1 = $t14; $t14 = $tmp;
  $tmp = $t2; $t2 = $t13; $t13 = $tmp;
  $tmp = $t3; $t3 = $t12; $t12 = $tmp;
  $tmp = $t4; $t4 = $t11; $t11 = $tmp;
  $tmp = $t5; $t5 = $t10; $t10 = $tmp;
  $tmp = $t6; $t6 = $t9; $t9 = $tmp;
  $tmp = $t7; $t7 = $t8; $t8 = $tmp;

  $sum = $sum + $t0;
  $sum = $sum + $t1;
  $sum = $sum + $t2;
  $sum = $sum + $t3;
  $sum = $sum + $t4;
  $sum = $sum + $t5;
  $sum = $sum + $t6;
  $sum = $sum + $t7;
  $sum = $sum + $t8;
  $sum = $sum + $t9;
  $sum = $sum + $t10;
  $sum = $sum + $t11;
  $sum = $sum + $t12;
  $sum = $sum + $t13;
  $sum = $sum + $t14;
  $sum = $sum + $t15;

  if ($sum != 0)
    echo $sum . "\n";
}

foo(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

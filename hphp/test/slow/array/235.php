<?php

function test($a) {
  var_dump(current($a));
  while (next($a)) echo '.';
  $a[] = 2;
  $a[] = 3;
  var_dump(current($a));
  var_dump(next($a));
  var_dump(next($a));
  var_dump(current($a));
  $a[17] = 4;
  var_dump(current($a));
  $a[18] = 5;
  var_dump(current($a));
  while(next($a)) echo '.';
  var_dump(current($a));
  $a[1] = 5;
  var_dump(current($a));
}
test(array(1));
test(array(1,2,3,4,5,6,7,8,9));

<?php

function test($s) {
  $a = array('abc' => 1, 'abcd' => 2);
  $s .= 'c';
 var_dump($a[$s]);
  $s .= 'd';
 var_dump($a[$s]);
}
test('ab');

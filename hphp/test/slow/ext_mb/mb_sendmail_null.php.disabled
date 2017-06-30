<?php

function test($n) {
  return mb_send_mail("A\x00".str_repeat('b', $n), '', '');
}

function main($n) {
  $a = str_repeat('a', $n + 2);
  $b = str_repeat('b', $n + 2);
  $c = str_repeat('c', $n + 2);
  $d = str_repeat('d', $n + 2);
  $b = '';
  $c ='';
  var_dump(test($n));
  var_dump($a, $d);
}

var_dump(test(32));

<?php

function main($v1, $v2, $v3, $v4, $v5, $v6) {
  $a = floor($v1);
  $b = ceil($v1);
  $c = floor($v2);
  $d = ceil($v2);
  $e = floor(5.5);
  $f = floor(-5.5);
  $g = ceil(5.5);
  $h = ceil(-5.5);
  $i = floor(2);
  $j = ceil(2);
  $k = floor(-2);
  $l = ceil(-2);
  $m = floor(0);
  $n = ceil(0);
  $o = floor(0.0);
  $p = ceil(-0.0);
  $q = floor($v3);
  $r = ceil($v3);
  $s = floor($v4);
  $t = ceil($v4);
  $u = floor($v5);
  $v = ceil($v5);
  $w = floor($v6);
  $x = ceil($v6);

  var_dump($a);
  var_dump($b);
  var_dump($c);
  var_dump($d);
  var_dump($e);
  var_dump($f);
  var_dump($g);
  var_dump($h);
  var_dump($i);
  var_dump($j);
  var_dump($k);
  var_dump($l);
  var_dump($m);
  var_dump($n);
  var_dump($o);
  var_dump($p);
  var_dump($q);
  var_dump($r);
  var_dump($s);
  var_dump($t);
  var_dump($u);
  var_dump($v);
  var_dump($w);
  var_dump($x);
}

main(3, -3, 3.5, -3.5, 0, 0.0);


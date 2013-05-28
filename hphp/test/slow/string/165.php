<?php

function test($a, $b) {
  $buf = 'hello';
  foreach ($a as $v) {
    $buf .= $v . ';';
    foreach ($b as $w) {
      $buf .= $w;
    }
  }
  var_dump($buf);
}
test(array('a', 'b', 'c'), array('u', 'v'));
function test2($a, $b) {
  $buf = 'hello';
  foreach ($a as $v) {
    $buf .= $v . ';';
    foreach ($b as $w) {
      $buf .= $w;
    }
    echo $buf;
  }
  var_dump($buf);
}
test2(array('a', 'b', 'c'), array('u', 'v'));
function test3($a, $b) {
  $buf = 'hello';
  foreach ($a as $v) {
    $buf .= $v . ';';
    foreach ($b as $w) {
      echo ($buf .= $w);
    }
  }
  var_dump($buf);
}
test3(array('a', 'b', 'c'), array('u', 'v'));

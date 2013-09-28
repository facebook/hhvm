<?php

function test($a, $b, $i) {
  list($a[$i++], $a[$i++], $a[$i++]) = $b;
  var_dump($a);
  }
test(array(), array('x', 'y', 'z'), 0);

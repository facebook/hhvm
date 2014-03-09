<?php

function aa() { return 'a'; }
function heh() { return array('a' => aa()); }
function x() {
  $x = heh();
  $r = $x[null] = 2;
  var_dump($r);
  var_dump($x);
}
x();

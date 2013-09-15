<?php

function test($x) {
  // each of these should "just work" but stresses NewPackedArray logic.
  $a = array($x, 1, 2); var_dump($a);
  $a = array($x,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1); var_dump($a);
  $a = array(0,$x,0,0,0,0,0,8); var_dump($a);
  $a = array(0,$x,0,0,0,0,0,0,0,0,11); var_dump($a);
  $a = array(0,$x,0,0,0,0,0,0,0,0,0,12); var_dump($a);
  $a = array(1,1,$x,"a"=>$x); var_dump($a);
}

test(42);


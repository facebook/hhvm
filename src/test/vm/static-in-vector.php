<?php

function bloo() {
  static $splonk = 1;

  $arr = array(2, array(2, array(2, 15213)));
  echo $arr[1][$splonk][1] . "\n";
}

bloo();

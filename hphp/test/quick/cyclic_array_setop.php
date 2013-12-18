<?php

// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);

function cyclic_array_setop() {
  $a = array();
  $a['x0'] = array("it's ");
  $a['x0'][0] =& $a['x0'];
  var_dump($a);
  var_dump($a['x0'][0] .= "ok");
}

cyclic_array_setop();

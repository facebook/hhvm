<?php


function g(&$x) {
  var_dump($x);
}

error_reporting(-1);
$b = array(20);
g(list($a) = $GLOBALS['b']);
g(list($a) = $b);


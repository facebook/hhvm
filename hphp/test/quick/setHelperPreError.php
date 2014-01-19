<?php

print "Test begin\n";

# BindM.
$v = array(0);
$w = 1;
$w[0] =& $v[0];
var_dump($v);
var_dump($w);

# FPassM.
function f(&$x) {
  var_dump($x);
}
$x = 1;
f($x[0]);
var_dump($x);

# VGetM.
$y = 1;
$z = &$y[0];
var_dump($z);

print "Test end\n";

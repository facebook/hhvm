<?php

$a['a'][][2] = 3;
var_dump($a);

$b = 4;
$b[5] = 6;
var_dump($b);

$c = array(true);
$c[0][1][0][0] = 7;
var_dump($c);

$x = null;
$x[0] = "x";
var_dump($x);

$s = "abc";
$s[-1] = "x";
$s[6] = "gx";
$s[4] = "ex";
var_dump($s);

// Special handling of a few types.
function t($val) {
  var_dump($val[] = 1);
}
t("");
t(false);
# t("nonempty"); # Causes fatal.
t(true);

<?php

$a = array();
$a[0] = 10;
$a[1] = 11;
$a["hi"] = "HI";
$a["bye"] = "BYE";
unset($a[1]);
unset($a["hi"]);
var_dump($a);

// Try out G bases as well.
$idxDefined = "foo";
$idxNotDefined = "-- )) \\";
$a[$idxDefined] = 071177;
var_dump($a);
unset($GLOBALS['a'][$idxDefined]);
unset($GLOBALS['a'][$idxNotDefined]);
var_dump($a);

// Regression test for a translator bug
$k = strtolower('blah');  // make it a dynamic string
$s = array($k => 123);
unset($s[$k]);
unset($s[$k]);  // should have no effect
var_dump($s);

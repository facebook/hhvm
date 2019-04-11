<?php
$foo = "ABC = DEF";
$fmt = "%s = %s %n";
$res_a0 = null;
$res_a1 = null;
$res_a2 = null;

/* $res_a[2] is supposed to be a integer value that
 * represents the number of characters consumed so far
 */
sscanf($foo, $fmt, &$res_a0, &$res_a1, &$res_a2);

$res_b = sscanf($foo, $fmt);

var_dump([$res_a0, $res_a1, $res_a2]);
var_dump($res_b);

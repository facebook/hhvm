<?php
$a = array();
$a[0] =& $a;
$a[1] = array();
$a[1][0] =& $a[1];
$a = 1;
var_dump(gc_collect_cycles());
echo "ok\n"
?>
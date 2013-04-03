<?php
$a = array();
$a[0] = new stdClass();
$a[0]->a = array();
$a[0]->a[0] =& $a[0];
var_dump($a[0]);
var_dump(gc_collect_cycles());
unset($a);
var_dump(gc_collect_cycles());
echo "ok\n"
?>
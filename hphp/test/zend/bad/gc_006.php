<?php
$a = new stdClass();
$a->a = array();
$a->a[0] =& $a;
var_dump($a);
unset($a);
var_dump(gc_collect_cycles());
echo "ok\n"
?>
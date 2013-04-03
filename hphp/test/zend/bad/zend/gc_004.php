<?php
$a = array();
$a[] =& $a;
var_dump($a);
unset($a);
var_dump(gc_collect_cycles());
echo "ok\n"
?>
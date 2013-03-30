<?php
$a = array();
$a[] =& $a;
var_dump($a);
$a[] =& $GLOBALS;
unset($a);
var_dump(gc_collect_cycles());
echo "ok\n"
?>
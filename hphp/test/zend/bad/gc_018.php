<?php
$a = array(array());
$a[0][0] =& $a[0];
$a = 1;
var_dump(gc_collect_cycles());
echo "ok\n"
?>
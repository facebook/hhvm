<?php
$a = array(array());
$a[0][0] =& $a[0];
$b = 1;
$a =& $b;
var_dump(gc_collect_cycles());
echo "ok\n"
?>
<?php
$a = array(array());
$a[0][0] =& $a[0];
$s = array(1) + unserialize(serialize($a[0]));
var_dump(gc_collect_cycles());
echo "ok\n"
?>

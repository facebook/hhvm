<?php
$a = new ArrayObject();
$a[0] = $a;
unset($a);
var_dump(gc_collect_cycles());
echo "ok\n";
?>
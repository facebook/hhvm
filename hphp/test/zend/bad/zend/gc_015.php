<?php
$a = new stdClass();
$c =& $a;
$b = $a;
$a->a = $a;
$a->b = "xxx";
unset($c);
unset($a);
unset($b);
var_dump(gc_collect_cycles());
echo "ok\n";
?>
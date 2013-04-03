<?php
$a = new stdClass();
$a->a = new stdClass();
$a->a->a = $a->a;
var_dump($a->a);
var_dump(gc_collect_cycles());
unset($a);
var_dump(gc_collect_cycles());
echo "ok\n"
?>
<?php
gc_enable();
$objs = array();
$objs[1] = new DateTime();
gc_collect_cycles();
unset($objs);
echo "OK\n";
?>
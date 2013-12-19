<?php
$a=array();
for ($i=0; $i < 1000; $i++) {
	$a[$i] = array(array());
	$a[$i][0] = & $a[$i];
}
var_dump(gc_collect_cycles());
unset($a);
var_dump(gc_collect_cycles());
echo "ok\n";
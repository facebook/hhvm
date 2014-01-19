<?php

$a = array(1,2,3);

$o = new stdclass;
$o->prop = "value";

$c = $a + $o;
var_dump($c);

echo "Done\n";
?>
<?php

$a = new stdclass;
$a->prop = $a;

var_dump($a);
var_dump(json_encode($a));

echo "Done\n";

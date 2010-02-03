<?php

$a = array();
$a[] = &$a;

var_dump($a);
var_dump(json_encode($a));

/* Break circular data structure to prevent memory leaks */
unset($a[0]);

echo "Done\n";

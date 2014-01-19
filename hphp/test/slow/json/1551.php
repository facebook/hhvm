<?php

$a = array();
$a[] = &$a;
var_dump($a);
var_dump(json_encode($a));

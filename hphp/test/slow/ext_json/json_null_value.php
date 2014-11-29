<?php

$a = json_decode('{"key":""}');
$b = json_encode($a);

var_dump($a);
var_dump($b);

<?php

$a = array(1.23456789e+34, 1E666, 1E666/1E666);
$e = json_encode($a);
var_dump($a);

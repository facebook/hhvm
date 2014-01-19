<?php

$a = function() { return 1; };
$b = clone $a;
var_dump($a());
var_dump($b());

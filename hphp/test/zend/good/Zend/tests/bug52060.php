<?php

$closure = function($a) { echo $a; };

var_dump(method_exists($closure, '__invoke')); // true

?>
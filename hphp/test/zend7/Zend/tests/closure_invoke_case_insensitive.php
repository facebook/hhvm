<?php

$inc = function(&$n) {
    $n++;
};

$n = 1;
$inc->__INVOKE($n);
var_dump($n);

?>

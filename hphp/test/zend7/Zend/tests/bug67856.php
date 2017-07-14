<?php
$array = [1, 2, 3];
var_dump(array_reduce($array, function(&$a, &$b) {
    return $a + $b;
}, 0));
?>

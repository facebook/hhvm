<?php
function &test(&$b) {
    $a =& $b;
    try {
        return $a;
    } finally {
        $a = 2;
    }
}
$x = 1;
$y =& test($x);
var_dump($y);
$x = 3;
var_dump($y);
?>

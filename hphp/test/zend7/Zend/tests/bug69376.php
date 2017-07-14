<?php
function &test() {
    $var = array();
    $var[] =& $var;

    return $var;
};

$a = test();
$b = $a;
$b[0] = 123;

print_r($a);
print_r($b);
?>

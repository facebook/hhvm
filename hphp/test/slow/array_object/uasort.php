<?php

// Comparison function
function cmp($a, $b) {
    if ($a == $b) {
        return 0;
    }
    return ($a < $b) ? -1 : 1;
}

// Array to be sorted
$array = array(
 'a' => 4,
 'b' => 8,
 'c' => -1,
 'd' => -9,
 'e' => 2,
 'f' => 5,
 'g' => 3,
 'h' => -4,
);
$arrayObject = new ArrayObject($array);
print_r($arrayObject);

// Sort and print the resulting array
$arrayObject->uasort('cmp');
print_r($arrayObject);

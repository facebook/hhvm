<?php

$array1 = [1, 2, 3];
$array2 = [1, 2, 3];

foreach ($array1 as $x) {
    foreach ($array2 as $y) {
        echo "$x.$y\n";
        continue 2;
    }
}

?>

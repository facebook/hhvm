<?php

$arrays = [
    [1, 2, 3],
    [4, 5, 6],
    [7, 8, 9],
];
var_dump(array_map(null, ...$arrays));

?>

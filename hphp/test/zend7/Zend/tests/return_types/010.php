<?php
function &foo(array &$in) : array {
    return null;
}

$array = [1, 2, 3];
var_dump(foo($array));


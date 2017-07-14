<?php

function test(&$ref1, &$ref2) {
    $ref1 += 42;
    $ref2 -= 42;
    return true;
}

$i = $j = 0;
var_dump(call_user_func('test', $i, $j));
var_dump($i, $j);

var_dump(call_user_func_array('test', [$i, $j]));
var_dump($i, $j);

$x =& $i; $y =& $j;
var_dump(call_user_func('test', $i, $j));
var_dump($i, $j);

var_dump(call_user_func_array('test', [$i, $j]));
var_dump($i, $j);

?>

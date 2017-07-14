<?php

function id($x) {
    return $x;
}

function &ref_id(&$x) {
    return $x;
}

$c = 'c';
$array = ['a', 'b', $c];

foreach(id($array) as &$v) {
    $v .= 'q';
}
var_dump($array);

foreach(ref_id($array) as &$v) {
    $v .= 'q';
}
var_dump($array);

?>

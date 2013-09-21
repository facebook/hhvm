<?php

$a = 'a';
$b = 'b';

foreach ([&$a, &$b] as &$value) {
    $value .= '-foo';
}

var_dump($a, $b);

?>
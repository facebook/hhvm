<?php

function &nop(&$var) {
    return $var;
}

function &gen(&$var) {
    yield nop($var);
}

$var = "foo";
$gen = gen($var);
foreach ($gen as &$varRef) {
    $varRef = "bar";
}

var_dump($var);

?>
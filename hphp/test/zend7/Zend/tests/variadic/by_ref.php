<?php

function test(&... $args) {
    $i = 0;
    foreach ($args as &$arg) {
        $arg = $i++;
    }
}

test();
test($a);
var_dump($a);
test($b, $c, $d);
var_dump($b, $c, $d);

?>

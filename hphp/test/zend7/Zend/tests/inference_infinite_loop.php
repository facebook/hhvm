<?php

function test() {
    $b = false;
    do {
        $a = $a + PHP_INT_MAX + 2;
        $a = 0;
    } while ($b);
}
test();

?>

<?php

function test() {
    $b = false;
    $x = (1<<53)+1;
    do {
        $x = 1.0 * ($x - (1<<53));
    } while ($b);
    return $x;
}
var_dump(test());

?>

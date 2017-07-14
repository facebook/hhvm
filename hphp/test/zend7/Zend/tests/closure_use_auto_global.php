<?php

function test() {
    $fn = function() use($GLOBALS) {
        var_dump($GLOBALS);
    };
    $fn();
}
test();

?>

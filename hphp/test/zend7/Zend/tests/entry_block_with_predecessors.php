<?php

function test() {
    while (true) {
        var_dump($a + 1);
        $a = 1;
        if (@$i++) {
            break;
        }
    }
}

function test2() {
    while (true) {
        $a = 42;
        if (@$i++ > 1) {
            break;
        }
        $a = new stdClass;
    }
}

test();
test2();

?>

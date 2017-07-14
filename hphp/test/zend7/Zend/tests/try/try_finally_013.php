<?php

function foo() {
    $array = [1, 2, $n = 3];
    foreach ($array as $value) {
        try {
            echo "try\n";
            return;
        } finally {
            echo "finally\n";
            return;
        }
    }
}

foo();
?>
===DONE===

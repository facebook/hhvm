<?php

function foo() {
    $array = [1, 2, $n = 3];
    foreach ($array as $value) {
        var_dump($value);
        try {
            try {
                foreach ($array as $_) {
                    return;
                }
            } finally {
                throw new Exception;
            }
        } catch (Exception $e) { }
    }
}

foo();
?>
===DONE===

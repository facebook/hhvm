<?php

function test() {
    try {
        throw new Exception(1);
    } finally {
        try {
            return 42;
        } finally {
            throw new Exception(2);
        }
    }
}

try {
    test();
} catch (Exception $e) {
    echo $e, "\n";
}

?>

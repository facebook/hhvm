<?php

function test() {
    try {
        throw new Exception(1);
    } catch (Exception $e) {
        try {
            throw new Exception(2);
        } finally {
        }
    }
}

try {
    test();
} catch (Exception $e) {
    echo $e, "\n";
}

?>

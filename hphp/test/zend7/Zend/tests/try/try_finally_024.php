<?php

function test() {
    try {
        throw new Exception(1);
    } finally {
        try {
            try {
            } finally {
                throw new Exception(2);
            }
        } catch (Exception $e) {}
        try {
        } finally {
            throw new Exception(3);
        }
    }
}

try {
    test();
} catch (Exception $e) {
    echo $e, "\n";
}
?>

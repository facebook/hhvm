<?php
function test() {
    try {
        throw new Exception(1);
    } finally {
        try {
            try {
                throw new Exception(2);
            } finally {
            }
        } catch (Exception $e) {
        }
    }
}

try {
    test();
} catch (Exception $e) {
    echo "caught {$e->getMessage()}\n";
}

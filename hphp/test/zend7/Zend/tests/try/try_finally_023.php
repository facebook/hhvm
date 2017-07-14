<?php

class Dtor {
    public function __destruct() {
        throw new Exception(2);
    }
}

function test() {
    try {
        throw new Exception(1);
    } finally {
        try {
            foreach ([new Dtor] as $v) {
                unset($v);
                return 42;
            }
        } catch (Exception $e) {
        }
    }
}

try {
    test();
} catch (Exception $e) {
    echo $e, "\n";
}

?>

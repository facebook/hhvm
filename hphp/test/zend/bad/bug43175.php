<?php

class foobar {
    public function __destruct() {
        throw new Exception();
    }
    public function __call($m, $a) {
        return $this;
    }
}
function foobar() {
    return new foobar();
}
try {
    foobar()->unknown();
} catch (Exception $e) {
    echo "__call via traditional factory should be caught\n";
}
?>
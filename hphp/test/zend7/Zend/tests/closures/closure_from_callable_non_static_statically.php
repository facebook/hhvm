<?php

class A {
    public function method() {
    }
}

try {
    $fn = Closure::fromCallable(['A', 'method']);
    $fn();
} catch (TypeError $e) {
    echo $e->getMessage(), "\n";
}

?>

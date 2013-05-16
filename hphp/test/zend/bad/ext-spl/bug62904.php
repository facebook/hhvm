<?php

class foo extends SplFixedArray {       
    public function __construct($size) {
    }
}

$x = new foo(2);

try {
    $z = clone $x;
} catch (Exception $e) {
    var_dump($e->getMessage());
}
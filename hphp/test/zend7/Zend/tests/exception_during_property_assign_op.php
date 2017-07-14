<?php

class Test {
    public function __get($name) {
        throw new Exception;
    }
}

$test = new Test;
try {
    $test->prop += 42;
} catch (Exception $e) {}
var_dump($test);

?>

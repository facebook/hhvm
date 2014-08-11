<?php
$reflection = new ReflectionClass('Generator');
try {
    $generator = $reflection->newInstanceWithoutConstructor();
    var_dump($generator);
} catch (Exception $e) {
    var_dump($e->getMessage());
}

$generator  = $reflection->newInstance();
var_dump($generator);
?>

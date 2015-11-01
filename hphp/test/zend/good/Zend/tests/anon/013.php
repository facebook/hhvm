<?php
// closure binding to anonymous class
$class = new class {};
$foo = function() {
    return $this;
};
$closure = Closure::bind($foo, $class, $class);
var_dump($closure());
?>

<?php

$fn = function() {
    try {
        new stdClass instanceof self;
    } catch (Error $e) {
        echo $e->getMessage(), "\n";
    }
};
$fn();

?>

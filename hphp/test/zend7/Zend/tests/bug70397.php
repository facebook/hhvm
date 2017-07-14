<?php

$f = function () {
    $this->value = true;
    yield $this->value;
};

var_dump($f->call(new class {})->current());

?>

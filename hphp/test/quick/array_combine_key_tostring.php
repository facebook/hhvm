<?php

class Foo { function __toString() { return 'Hello'; } }
$foos = [new Foo];
$bar = array_combine($foos, $foos);

var_dump($bar);

?>

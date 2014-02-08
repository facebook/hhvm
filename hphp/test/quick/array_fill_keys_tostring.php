<?php

class Foo { function __toString() { return 'Hello'; } }
$foos = [new Foo];
var_dump(array_fill_keys($foos, "val"));

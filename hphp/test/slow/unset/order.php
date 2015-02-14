<?php

class blob {
    function __destruct() {
        echo "Running destructor\n";
        global $b;
        $b = $this;
    }
}

$b = new blob;
var_dump($b);
unset($b);
var_dump(isset($b));
unset($b);
var_dump(isset($b));

$name = 'b';
$$name = new blob;
var_dump($b);
unset($$name);
var_dump(isset($b));
unset($$name);
var_dump(isset($b));

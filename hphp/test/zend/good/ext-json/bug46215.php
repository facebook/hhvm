<?php

class foo {
    protected $a = array();
}

$a = new foo;
$x = json_encode($a);

print_r($a);

?>
<?php

class Bar extends ArrayObject {
    private $foo = array( 1, 2, 3 );
    function __construct()
    {
        parent::__construct($this->foo);
    }
}

$foo = new Bar();
var_dump($foo);
$foo['foo'] = 23;

$bar = new Bar();
var_dump($bar);

echo "Done\n";
?>
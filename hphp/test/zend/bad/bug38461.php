<?php

class Operation
{
    function __set( $var, $value )
    {
        $this->$var = $value;
    }
}

class ExtOperation extends Operation
{
    private $x;
}

$op = new ExtOperation;
$op->x = 'test';

echo "Done\n";
?>
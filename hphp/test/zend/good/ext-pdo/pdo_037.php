<?php

class MyStatement extends PDOStatement
{
}

$obj = new MyStatement;
var_dump($obj->foo());

?>
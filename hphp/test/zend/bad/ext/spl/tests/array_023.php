<?php

class Name extends ArrayObject
{
    public $var = 'a';
    protected $bar = 'b';
    private $foo = 'c';
}

$a = new Name();
var_dump($a);
var_dump($a->var);

$a = unserialize(serialize($a));

var_dump($a);
var_dump($a->var);

class Bla extends ArrayObject
{
    public $var = 'aaa';
    protected $bar = 'bbb';
    private $foo = 'ccc';
}

$a = new Bla();
var_dump($a);
var_dump($a->var);

$a = unserialize(serialize($a));

var_dump($a);
var_dump($a->var);

?>
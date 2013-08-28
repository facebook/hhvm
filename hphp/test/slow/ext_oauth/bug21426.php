<?php

class Foo extends OAuth
{
    protected $foo = 'bar';

    public function getFoo()
    {
        return $this->foo;
    }
}

$foo = new Foo('key', 'secret');
var_dump($foo->getFoo());

?>

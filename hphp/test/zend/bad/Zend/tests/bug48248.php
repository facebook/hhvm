<?php

class A
{
    public function & __get($name)
    {
        return $this->test;
    }
}

class B extends A
{
    private $test;
}

$b = new B;
var_dump($b->test);

?>
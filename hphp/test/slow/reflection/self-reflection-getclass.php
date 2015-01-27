<?php

class Foo
{
    public function compare(self $otherFoo) { return $otherFoo === $this; }
}

$rc = (new ReflectionParameter(['Foo', 'compare'], 'otherFoo'))->getClass();
var_dump(get_class($rc));
var_dump($rc->getName());

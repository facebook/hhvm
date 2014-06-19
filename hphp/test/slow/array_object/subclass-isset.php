<?php

class MyArrayObject extends ArrayObject
{
    public function offsetGet($index)
    {
        echo "offsetGet Called!\n";
        if (isset($this[$index])) { // infinite recursion in hhvm
            return parent::offsetGet($index);
        }

        return null;
    }
}
$obj = new MyArrayObject(['foo' => true]);

var_dump($obj['foo']);
var_dump(isset($obj['foo']));
var_dump(isset($obj['bar']));

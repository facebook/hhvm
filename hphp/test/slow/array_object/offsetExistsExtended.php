<?php

class TestArray extends ArrayObject
{
    public function offsetExists($index)
    {
        return true;
    }
}

$test = new TestArray();
var_dump(isset($test['abc']));

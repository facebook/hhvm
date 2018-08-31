<?php

class TestArray extends ArrayObject
{
    public function offsetExists($index)
    {
        return true;
    }
}


<<__EntryPoint>>
function main_offset_exists_extended() {
$test = new TestArray();
var_dump(isset($test['abc']));
}

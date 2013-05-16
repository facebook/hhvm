<?php

class MyFixedArray extends SplFixedArray {
    public function offsetGet($offset) {}
}

$array = new MyFixedArray(10);
$array[][1] = 10;

?>
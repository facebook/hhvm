<?php

class obj extends SplFixedArray{
    public function offsetSet($offset, $value) {
        var_dump($offset);
    }
}
<<__EntryPoint>> function main() {
$obj = new obj;

$obj[]=2;
$obj[]=2;
$obj[]=2;
}

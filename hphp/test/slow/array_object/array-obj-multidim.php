<?php

# case 1 - segmentation fault
class Concrete
{
    public function __construct($c)
    {
        $c['foo'] = array();
        $c['foo']['bar'] = 'hello!!';
        var_dump($c); //segmentation fault
    }
}

# case 2 - segmentation fault
function x() {
    $c = new ArrayObject;
    $c['foo'] = array();
    $c['foo']['bar'] = 'hello!!';
    var_dump($c); // segmentation fault
}

<<__EntryPoint>>
function main_array_obj_multidim() {
new Concrete(new ArrayObject());
x();

# case 3
$c = new ArrayObject;
$c['foo'] = array();
$c['foo']['bar'] = 'hello!!';
var_dump($c); // no errors
}

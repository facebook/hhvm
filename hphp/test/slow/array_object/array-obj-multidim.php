<?php

# case 1 - segmentation fault
class Concrete
{
    public function __construct($c)
    {
        $c['foo']['bar'] = 'hello!!';
        var_dump($c); //segmentation fault
    }
}
new Concrete(new ArrayObject());

# case 2 - segmentation fault
function x() {
    $c = new ArrayObject;
    $c['foo']['bar'] = 'hello!!';
    var_dump($c); // segmentation fault
}
x();

# case 3
$c = new ArrayObject;
$c['foo']['bar'] = 'hello!!';
var_dump($c); // no errors

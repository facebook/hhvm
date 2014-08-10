<?php

class A
{
    protected $value;

    public function __construct($val)
    {
        $this->value = $val;
    }

    protected function getValue()
    {
        return $this->value;
    }
}

class B extends A
{
    public function copyValue($obj)
    {
        $this->value = $obj->getValue();
        $this->value = $obj->value; // value defined in common base class
    }
}
class C extends A {}

$B = new B("B");
var_dump($B);
$C = new C("C");
var_dump($C);

$B->copyValue($C);

var_dump($B);

?>
===DONE===

<?php
class A
{
    public function &getA()
    {
        return $this->a;
    }
}

$A = new A;
$A->a = array(1);
$x = $A->getA();
$clone = clone $A;
$clone->a = array();
print_r($A);
?>
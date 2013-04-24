<?php
class C {
        public $prop = 'C::prop.orig';
}
class MyArrayObject extends ArrayObject {
}
$c = new C;
$ao = new MyArrayObject($c);
testAccess($c, $ao);
function testAccess($c, $ao) {
        foreach ($ao as $key=>$value) {
        }
        unset($ao['prop']);
        var_dump($c->prop, $ao['prop']);
}
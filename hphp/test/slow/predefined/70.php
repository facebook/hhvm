<?php

class A {
 function TestR() {
 var_dump(__CLASS__, __METHOD__);
}
 static function Testm() {
 var_dump(__CLASS__, __METHOD__);
}
}
 function Testf() {
 var_dump(__CLASS__, __METHOD__);
}
 testf();
 A::testm();
 $obj = new A();
 $obj->testr();

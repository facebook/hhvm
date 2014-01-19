<?php

$global = B::CLASS_CONSTANT;
 $another = test2($global);
 define('CONSTANT', test2('defining'));
 function test2($a) {
 var_dump($a);
 return 12345;
}
 class A extends B {
}
 class B {
 const CLASS_CONSTANT = 1;
}


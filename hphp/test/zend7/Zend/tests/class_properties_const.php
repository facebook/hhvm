<?php
class A {
}

$a = new A;

echo "runtime\n";
var_dump($a->{array()});
var_dump($a->{1});
var_dump($a->{function(){}});
?>

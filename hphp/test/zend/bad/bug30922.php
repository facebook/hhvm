<?php
interface RecurisiveFooFar extends RecurisiveFooFar {}
class A implements RecurisiveFooFar {}

$a = new A();
var_dump($a instanceOf A);
echo "ok\n";
?>
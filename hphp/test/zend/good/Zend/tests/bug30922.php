<?php
interface RecurisiveFooFar extends RecurisiveFooFar {}
class A implements RecurisiveFooFar {}
<<__EntryPoint>> function main() {
$a = new A();
var_dump($a instanceOf A);
echo "ok\n";
}

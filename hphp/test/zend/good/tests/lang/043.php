<?php
class A {
    static function foo() { return 'foo'; }
}

$classname       =  'A';
$wrongClassname  =  'B';

echo $classname::foo()."\n";
echo $wrongClassname::foo()."\n";
?>
===DONE===
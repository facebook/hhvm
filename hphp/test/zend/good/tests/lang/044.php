<?php
class A {
    static function foo() { return 'foo'; }
}
$classname        =  'A';
$wrongClassname   =  'B';

$methodname       =  'foo';

echo $classname::$methodname()."\n";

echo $wrongClassname::$methodname()."\n";
?>
===DONE===
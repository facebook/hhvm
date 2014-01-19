<?php
class A {
    public    static $b = 'foo';
}

$classname       =  'A';
$wrongClassname  =  'B';

echo $classname::$b."\n";
echo $wrongClassname::$b."\n";

?>
===DONE===
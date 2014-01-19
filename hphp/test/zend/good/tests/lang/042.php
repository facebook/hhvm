<?php
class A {
    const B = 'foo';
}

$classname       =  'A';
$wrongClassname  =  'B';

echo $classname::B."\n";
echo $wrongClassname::B."\n";
?>
===DONE===
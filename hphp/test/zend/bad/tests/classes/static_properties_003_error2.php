<?php
class C {
    protected static $y = 'C::$y';
}
$c = new C;

echo "\n--> Access non-visible static prop like instance prop:\n";
echo $c->y;
?>
==Done==
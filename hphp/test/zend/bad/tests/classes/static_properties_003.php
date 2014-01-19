<?php
class C {
    public static $x = 'C::$x';
    protected static $y = 'C::$y';
}

$c = new C;

echo "\n--> Access visible static prop like instance prop:\n";
var_dump(isset($c->x));
unset($c->x);
echo $c->x;
$c->x = 1;
$ref = 'ref';
$c->x =& $ref;
var_dump($c->x, C::$x);

echo "\n--> Access non-visible static prop like instance prop:\n";
var_dump(isset($c->y));
//unset($c->y);		// Fatal error, tested in static_properties_003_error1.phpt
//echo $c->y;		// Fatal error, tested in static_properties_003_error2.phpt
//$c->y = 1;		// Fatal error, tested in static_properties_003_error3.phpt
//$c->y =& $ref;	// Fatal error, tested in static_properties_003_error4.phpt
?>
==Done==
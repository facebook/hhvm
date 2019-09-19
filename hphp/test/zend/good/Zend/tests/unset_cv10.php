<?hh
$a = new ArrayObject($GLOBALS['GLOBALS']);
$x = "ok\n";
echo $x;
$a->offsetUnset('x');
echo $x;
echo "ok\n";

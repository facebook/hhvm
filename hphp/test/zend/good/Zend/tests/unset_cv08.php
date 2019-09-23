<?hh
$a = "ok\n";
$b = "ok\n";
@array_unique($GLOBALS['GLOBALS']);
echo $a;
echo $b;
echo "ok\n";
# NOTE: The assignments above must run at top-level.

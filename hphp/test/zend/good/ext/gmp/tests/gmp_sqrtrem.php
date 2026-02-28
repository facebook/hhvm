<?hh
<<__EntryPoint>> function main(): void {
$r = gmp_sqrtrem(-1);
var_dump(gmp_strval($r[0]));
var_dump(gmp_strval($r[1]));

$r = gmp_sqrtrem("0");
var_dump(gmp_strval($r[0]));
var_dump(gmp_strval($r[1]));

$r = gmp_sqrtrem(2);
var_dump(gmp_strval($r[0]));
var_dump(gmp_strval($r[1]));

$r = gmp_sqrtrem(10);
var_dump(gmp_strval($r[0]));
var_dump(gmp_strval($r[1]));

$r = gmp_sqrtrem(7);
var_dump(gmp_strval($r[0]));
var_dump(gmp_strval($r[1]));

$r = gmp_sqrtrem(3);
var_dump(gmp_strval($r[0]));
var_dump(gmp_strval($r[1]));

$r = gmp_sqrtrem(100000);
var_dump(gmp_strval($r[0]));
var_dump(gmp_strval($r[1]));

$r = gmp_sqrtrem("1000000");
var_dump(gmp_strval($r[0]));
var_dump(gmp_strval($r[1]));

$r = gmp_sqrtrem("1000001");
var_dump(gmp_strval($r[0]));
var_dump(gmp_strval($r[1]));


$n = gmp_init(-1);
$r = gmp_sqrtrem($n);
var_dump(gmp_strval($r[0]));
var_dump(gmp_strval($r[1]));

$n = gmp_init(1000001);
$r = gmp_sqrtrem($n);
var_dump(gmp_strval($r[0]));
var_dump(gmp_strval($r[1]));

var_dump(gmp_sqrtrem(vec[]));
try { var_dump(gmp_sqrtrem()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}

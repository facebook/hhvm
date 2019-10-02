<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(gmp_testbit()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

$n = gmp_init(0);
var_dump(gmp_testbit($n, -10));
var_dump(gmp_testbit($n, 0));
var_dump(gmp_testbit($n, 1));
var_dump(gmp_testbit($n, 100));

$n = gmp_init(-1);
var_dump(gmp_testbit($n, 1));
var_dump(gmp_testbit($n, -1));

$n = gmp_init("1000000");
var_dump(gmp_testbit($n, 1));
gmp_setbit(inout $n, 1);
var_dump(gmp_testbit($n, 1));
var_dump(gmp_strval($n));

gmp_setbit(inout $n, 5);
var_dump(gmp_testbit($n, 5));
var_dump(gmp_strval($n));

$n = gmp_init("238462734628347239571823641234");
var_dump(gmp_testbit($n, 5));
gmp_setbit(inout $n, 5);
var_dump(gmp_testbit($n, 5));
var_dump(gmp_strval($n));

gmp_clrbit(inout $n, 5);
var_dump(gmp_testbit($n, 5));
var_dump(gmp_strval($n));

echo "Done\n";
}

<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(gmp_intval(1,1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_intval(""));
var_dump(gmp_intval(1.0001));
var_dump(gmp_intval("1.0001"));
var_dump(gmp_intval("-1"));
var_dump(gmp_intval(-1));
var_dump(gmp_intval(-2349828));
var_dump(gmp_intval(2342344));
var_dump(gmp_intval(vec[]));

$fp = fopen(__FILE__, 'r');
var_dump(gmp_intval($fp));

$g = gmp_init("12345678");
var_dump(gmp_intval($g));

echo "Done\n";
}

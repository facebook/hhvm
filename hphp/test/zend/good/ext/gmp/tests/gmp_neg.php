<?hh
<<__EntryPoint>> function main(): void {
var_dump(gmp_intval(gmp_neg(0)));
var_dump(gmp_intval(gmp_neg(1)));
var_dump(gmp_intval(gmp_neg(-1)));
var_dump(gmp_intval(gmp_neg("-1")));
var_dump(gmp_intval(gmp_neg("")));
var_dump(gmp_intval(gmp_neg(0)));

$n = gmp_init("0");
var_dump(gmp_intval(gmp_neg($n)));
$n = gmp_init("12345678901234567890");
var_dump(gmp_strval(gmp_neg($n)));

try { var_dump(gmp_neg(1,1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_neg()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_neg(vec[]));

echo "Done\n";
}

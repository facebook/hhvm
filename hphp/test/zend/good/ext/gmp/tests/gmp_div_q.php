<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(gmp_div_q()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_div_q("")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(gmp_div_q(0,1));
var_dump(gmp_div_q(1,0));
var_dump(gmp_div_q(12653,23482734));
var_dump(gmp_div_q(12653,23482734, 10));
var_dump(gmp_div_q(1123123,123));
var_dump(gmp_div_q(1123123,123, 1));
var_dump(gmp_div_q(1123123,123, 2));
var_dump(gmp_div_q(1123123,123, GMP_ROUND_ZERO));
var_dump(gmp_div_q(1123123,123, GMP_ROUND_PLUSINF));
var_dump(gmp_div_q(1123123,123, GMP_ROUND_MINUSINF));

$fp = fopen(__FILE__, 'r');

var_dump(gmp_div_q($fp, $fp));
var_dump(gmp_div_q(vec[], vec[]));

echo "Done\n";
}

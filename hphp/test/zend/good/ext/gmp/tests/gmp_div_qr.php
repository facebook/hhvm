<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(gmp_div_qr()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_div_qr("")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(gmp_div_qr(0,1));
var_dump(gmp_div_qr(1,0));
var_dump(gmp_div_qr(gmp_init(1), gmp_init(0)));
var_dump(gmp_div_qr(12653,23482734));
var_dump(gmp_div_qr(12653,23482734, 10));
var_dump(gmp_div_qr(1123123,123));
var_dump(gmp_div_qr(1123123,123, 1));
var_dump(gmp_div_qr(1123123,123, 2));
var_dump(gmp_div_qr(gmp_init(1123123), gmp_init(123)));
var_dump(gmp_div_qr(1123123,123, GMP_ROUND_ZERO));
var_dump(gmp_div_qr(1123123,123, GMP_ROUND_PLUSINF));
var_dump(gmp_div_qr(1123123,123, GMP_ROUND_MINUSINF));

$fp = fopen(__FILE__, 'r');

var_dump(gmp_div_qr($fp, $fp));
var_dump(gmp_div_qr(vec[], vec[]));

echo "Done\n";
}

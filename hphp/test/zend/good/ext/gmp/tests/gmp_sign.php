<?hh
<<__EntryPoint>> function main(): void {
var_dump(gmp_sign(-1));
var_dump(gmp_sign(1));
var_dump(gmp_sign(0));
var_dump(gmp_sign("123718235123123"));
var_dump(gmp_sign("-34535345345"));
var_dump(gmp_sign("+34534573457345"));
$n = gmp_init("098909878976786545");
var_dump(gmp_sign($n));
try { var_dump(gmp_sign($n, $n)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_sign(vec[]));
try { var_dump(gmp_sign()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}

<?hh
<<__EntryPoint>> function main(): void {
var_dump(gmp_strval(gmp_abs("")));
var_dump(gmp_strval(gmp_abs("0")));
var_dump(gmp_strval(gmp_abs(0)));
var_dump(gmp_strval(gmp_abs(-1111111111111111111)));
var_dump(gmp_strval(gmp_abs("111111111111111111111")));
var_dump(gmp_strval(gmp_abs("-111111111111111111111")));
var_dump(gmp_strval(gmp_abs("0000")));
var_dump(gmp_strval(gmp_abs("09876543")));
var_dump(gmp_strval(gmp_abs("-099987654")));

try { var_dump(gmp_abs()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_abs(1,2)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_abs(vec[]));

echo "Done\n";
}

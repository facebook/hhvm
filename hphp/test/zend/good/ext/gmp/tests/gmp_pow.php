<?hh
<<__EntryPoint>> function main(): void {
var_dump(gmp_strval(gmp_pow(2,10)));
var_dump(gmp_strval(gmp_pow(-2,10)));
var_dump(gmp_strval(gmp_pow(-2,11)));
var_dump(gmp_strval(gmp_pow("2",10)));
var_dump(gmp_strval(gmp_pow("2",0)));
var_dump(gmp_strval(gmp_pow("2",-1)));
var_dump(gmp_strval(gmp_pow("-2",10)));
var_dump(gmp_strval(gmp_pow(20,10)));
var_dump(gmp_strval(gmp_pow(50,10)));
var_dump(gmp_strval(gmp_pow(50,-5)));

$n = gmp_init("20");
var_dump(gmp_strval(gmp_pow($n,10)));
$n = gmp_init("-20");
var_dump(gmp_strval(gmp_pow($n,10)));

try { var_dump(gmp_pow(2,10,1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_pow(2)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_pow()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_pow(vec[], vec[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_pow(2,vec[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_pow(vec[],10));

echo "Done\n";
}

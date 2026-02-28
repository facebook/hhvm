<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(gmp_mod()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_mod("")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_mod("",""));
var_dump(gmp_mod(0,1));
var_dump(gmp_mod(0,-1));
var_dump(gmp_mod(-1,0));

var_dump(gmp_mod(vec[], vec[]));

$a = gmp_init("-100000000");
$b = gmp_init("353467");

var_dump(gmp_mod($a, $b));

echo "Done\n";
}

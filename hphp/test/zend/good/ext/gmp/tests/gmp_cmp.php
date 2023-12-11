<?hh
<<__EntryPoint>> function main(): void {
var_dump(gmp_cmp(123123,-123123));
var_dump(gmp_cmp("12345678900987654321","12345678900987654321"));
var_dump(gmp_cmp("12345678900987654321","123456789009876543211"));
var_dump(gmp_cmp(0,0));
var_dump(gmp_cmp(1231222,0));
var_dump(gmp_cmp(0,345355));

$n = gmp_init("827278512385463739");
var_dump(gmp_cmp(0,$n) < 0);
$n1 = gmp_init("827278512385463739");
var_dump(gmp_cmp($n1,$n));

try { var_dump(gmp_cmp($n1,$n,1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_cmp(vec[],vec[]));
try { var_dump(gmp_cmp(vec[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_cmp()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}

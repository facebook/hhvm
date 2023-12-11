<?hh
<<__EntryPoint>> function main(): void {
var_dump(gmp_strval(gmp_sqrt(-2)));
var_dump(gmp_strval(gmp_sqrt("-2")));
var_dump(gmp_strval(gmp_sqrt("0")));
var_dump(gmp_strval(gmp_sqrt("2")));
var_dump(gmp_strval(gmp_sqrt("144")));

$n = gmp_init(0);
var_dump(gmp_strval(gmp_sqrt($n)));
$n = gmp_init(-144);
var_dump(gmp_strval(gmp_sqrt($n)));
$n = gmp_init(777);
var_dump(gmp_strval(gmp_sqrt($n)));

try { var_dump(gmp_sqrt($n, 1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_sqrt()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_sqrt(vec[]));

echo "Done\n";
}

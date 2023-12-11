<?hh
<<__EntryPoint>> function main(): void {
var_dump(gmp_strval(gmp_com(0)));
var_dump(gmp_strval(gmp_com("0")));
var_dump(gmp_strval(gmp_com("test")));
var_dump(gmp_strval(gmp_com("2394876545678")));
var_dump(gmp_strval(gmp_com("-111")));
var_dump(gmp_strval(gmp_com(874653)));
var_dump(gmp_strval(gmp_com(-9876)));

$n = gmp_init("98765467");
var_dump(gmp_strval(gmp_com($n)));
$n = gmp_init("98765463337");
var_dump(gmp_strval(gmp_com($n)));

var_dump(gmp_strval(gmp_com(vec[])));
try { var_dump(gmp_strval(gmp_com())); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}

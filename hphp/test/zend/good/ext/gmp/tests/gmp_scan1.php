<?hh
<<__EntryPoint>> function main(): void {
var_dump(gmp_scan1("434234", -10));
var_dump(gmp_scan1("434234", 1));
var_dump(gmp_scan1(4096, 0));
var_dump(gmp_scan1("1000000000", 5));
var_dump(gmp_scan1("1000000000", 200));

$n = gmp_init("24234527465274");
var_dump(gmp_scan1($n, 10));

var_dump(gmp_scan1(vec[], 200));
try { var_dump(gmp_scan1(vec[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_scan1()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}

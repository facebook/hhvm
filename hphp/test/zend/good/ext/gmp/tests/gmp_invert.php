<?hh
<<__EntryPoint>> function main(): void {
var_dump(gmp_strval(gmp_invert(123123,5467624)));
var_dump(gmp_strval(gmp_invert(123123,"3333334345467624")));
var_dump(gmp_strval(gmp_invert("12312323213123123",7624)));
var_dump(gmp_strval(gmp_invert(444,0)));
var_dump(gmp_strval(gmp_invert(0,28347)));
var_dump(gmp_strval(gmp_invert(-12,456456)));
var_dump(gmp_strval(gmp_invert(234234,-435345)));

$n = gmp_init("349827349623423452345");
$n1 = gmp_init("3498273496234234523451");

var_dump(gmp_strval(gmp_invert($n, $n1)));
var_dump(gmp_strval(gmp_invert($n1, $n)));

try { var_dump(gmp_invert($n1, $n, 10)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_invert($n1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_invert(vec[], 1));
var_dump(gmp_invert(1, vec[]));
var_dump(gmp_invert(vec[], vec[]));

echo "Done\n";
}

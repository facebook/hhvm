<?hh
<<__EntryPoint>> function main(): void {
var_dump(gmp_strval(gmp_fact(0)));
var_dump(gmp_strval(gmp_fact("")));
var_dump(gmp_strval(gmp_fact("0")));
var_dump(gmp_strval(gmp_fact("-1")));
var_dump(gmp_strval(gmp_fact(-1)));
var_dump(gmp_strval(gmp_fact(1.1)));
var_dump(gmp_strval(gmp_fact(20)));
var_dump(gmp_strval(gmp_fact("50")));
var_dump(gmp_strval(gmp_fact("10")));
var_dump(gmp_strval(gmp_fact("0000")));

$n = gmp_init(12);
var_dump(gmp_strval(gmp_fact($n)));
$n = gmp_init(-10);
var_dump(gmp_strval(gmp_fact($n)));

try { var_dump(gmp_fact()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_fact(1,1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_fact(vec[]));
var_dump(gmp_strval(gmp_fact(vec[])));

echo "Done\n";
}

<?hh
<<__EntryPoint>> function main(): void {
$n = gmp_init(-1);
gmp_setbit(inout $n, 10, true);
var_dump(gmp_strval($n));

$n = gmp_init(5);
gmp_setbit(inout $n, -20, false);
var_dump(gmp_strval($n));

$n = gmp_init(5);
gmp_setbit(inout $n, 2, false);
var_dump(gmp_strval($n));

$n = gmp_init(5);
gmp_setbit(inout $n, 1, true);
var_dump(gmp_strval($n));

$n = gmp_init("100000000000");
gmp_setbit(inout $n, 23, true);
var_dump(gmp_strval($n));

gmp_setbit(inout $n, 23, false);
var_dump(gmp_strval($n));

gmp_setbit(inout $n, 3);
var_dump(gmp_strval($n));

$b = "";
gmp_setbit(inout $b, 23);
try { gmp_setbit(inout $b); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { gmp_setbit(inout $b, 23,1,1); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { gmp_setbit(inout $b,vec[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
$a = vec[];
try { gmp_setbit(inout $a,vec[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}

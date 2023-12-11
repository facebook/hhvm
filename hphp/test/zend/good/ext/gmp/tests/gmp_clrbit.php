<?hh
<<__EntryPoint>> function main(): void {
$n = gmp_init(0);
gmp_clrbit(inout $n, 0);
var_dump(gmp_strval($n));

$n = gmp_init(-1);
gmp_clrbit(inout $n, -1);
var_dump(gmp_strval($n));

$n = gmp_init("1000000");
gmp_clrbit(inout $n, -1);
var_dump(gmp_strval($n));

$n = gmp_init("1000000");
gmp_clrbit(inout $n, 3);
var_dump(gmp_strval($n));

$n = gmp_init("238462734628347239571823641234");
gmp_clrbit(inout $n, 3);
gmp_clrbit(inout $n, 5);
gmp_clrbit(inout $n, 20);
var_dump(gmp_strval($n));

$n = vec[];
gmp_clrbit(inout $n, 3);
try { gmp_clrbit(inout $n, 3, 1); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { gmp_clrbit(inout $n); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { gmp_clrbit(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}

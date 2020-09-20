<?hh
<<__EntryPoint>> function main(): void {
var_dump(gmp_strval(gmp_powm(0,1,10)));
var_dump(gmp_strval(gmp_powm(5,1,10)));
var_dump(gmp_strval(gmp_powm(-5,1,-10)));
var_dump(gmp_strval(gmp_powm(-5,1,10)));
var_dump(gmp_strval(gmp_powm(-5,11,10)));
var_dump(gmp_strval(gmp_powm("77",3,1000)));

$n = gmp_init(11);
var_dump(gmp_strval(gmp_powm($n,3,1000)));
$e = gmp_init(7);
var_dump(gmp_strval(gmp_powm($n,$e,1000)));
$m = gmp_init(900);
var_dump(gmp_strval(gmp_powm($n,$e,$m)));

var_dump(gmp_powm(5, 11, 0));
var_dump(gmp_powm(5, "11", gmp_init(0)));

var_dump(gmp_powm(varray[],$e,$m));
var_dump(gmp_powm($n,varray[],$m));
var_dump(gmp_powm($n,$e,varray[]));
var_dump(gmp_powm(varray[],varray[],varray[]));
try { var_dump(gmp_powm(varray[],varray[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_powm(varray[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_powm()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

$n = gmp_init("-5");
var_dump(gmp_powm(10, $n, 10));

$n = gmp_init("0");
var_dump(gmp_powm(10, $n, 10));

echo "Done\n";
}

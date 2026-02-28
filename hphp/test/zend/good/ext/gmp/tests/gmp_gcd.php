<?hh
<<__EntryPoint>> function main(): void {
var_dump(gmp_strval(gmp_gcd(234,12387)));
var_dump(gmp_strval(gmp_gcd(0,12387)));
var_dump(gmp_strval(gmp_gcd(224,0)));
var_dump(gmp_strval(gmp_gcd(-1,0)));
var_dump(gmp_strval(gmp_gcd(-1,0)));
var_dump(gmp_strval(gmp_gcd("12371238123","32618723123")));
var_dump(gmp_strval(gmp_gcd("7623456735","12372341234")));

$n = gmp_init("8127346234");
var_dump(gmp_strval(gmp_gcd($n,"12372341234")));
$n = gmp_init("8127346234");
var_dump(gmp_strval(gmp_gcd("7623456735",$n)));
$n = gmp_init("8127346234");
var_dump(gmp_strval(gmp_gcd($n,$n)));
$n = gmp_init("8127346234");
var_dump(gmp_strval(gmp_gcd($n,0)));
try { var_dump(gmp_gcd($n,$n,1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_gcd($n,vec[],1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_gcd(vec[],$n,1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}

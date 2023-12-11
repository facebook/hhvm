<?hh
<<__EntryPoint>> function main(): void {
$n = gmp_init("34293864345");
$n1 = gmp_init("23434293864345");

$a = vec[
	vec[123,45],
	vec[4341,9734],
	vec[23487,333],
	vec[-234234,-123123],
	vec[-100,-2234],
	vec[345,"34587345"],
	vec[345,"0"],
	vec["345556456",345873],
	vec["34545345556456","323432445873"],
	vec[$n, $n1],
	];

foreach ($a as $val) {
	$r = gmp_gcdext($val[0],$val[1]);
	var_dump(gmp_strval($r['g']));
	var_dump(gmp_strval($r['s']));
	var_dump(gmp_strval($r['t']));
}

var_dump(gmp_gcdext($val[0],vec[]));
var_dump(gmp_gcdext(vec[],vec[]));
try { var_dump(gmp_gcdext(vec[],vec[],1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_gcdext(vec[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_gcdext()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}

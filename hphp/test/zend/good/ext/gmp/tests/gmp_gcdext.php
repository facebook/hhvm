<?hh
<<__EntryPoint>> function main(): void {
$n = gmp_init("34293864345");
$n1 = gmp_init("23434293864345");

$a = varray[
	varray[123,45],
	varray[4341,9734],
	varray[23487,333],
	varray[-234234,-123123],
	varray[-100,-2234],
	varray[345,"34587345"],
	varray[345,"0"],
	varray["345556456",345873],
	varray["34545345556456","323432445873"],
	varray[$n, $n1],
	];

foreach ($a as $val) {
	$r = gmp_gcdext($val[0],$val[1]);
	var_dump(gmp_strval($r['g']));
	var_dump(gmp_strval($r['s']));
	var_dump(gmp_strval($r['t']));
}

var_dump(gmp_gcdext($val[0],varray[]));
var_dump(gmp_gcdext(varray[],varray[]));
try { var_dump(gmp_gcdext(varray[],varray[],1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_gcdext(varray[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_gcdext()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}

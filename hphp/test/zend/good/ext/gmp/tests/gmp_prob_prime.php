<?hh
<<__EntryPoint>> function main(): void {
var_dump(gmp_prob_prime(10));
var_dump(gmp_prob_prime("7"));
var_dump(gmp_prob_prime(17));
var_dump(gmp_prob_prime(-31));
var_dump(gmp_prob_prime("172368715471481723"));

var_dump(gmp_prob_prime(10));
var_dump(gmp_prob_prime("7"));
var_dump(gmp_prob_prime(17));
var_dump(gmp_prob_prime(-31));
var_dump(gmp_prob_prime("172368715471481723"));

for ($i = -1; $i < 12; $i++) {
	var_dump(gmp_prob_prime((773*$i)-($i*7)-1, $i));
	$n = gmp_init("23476812735411");
	var_dump(gmp_prob_prime(gmp_add($n, $i-1), $i));
}

$n = gmp_init("19481923");
var_dump(gmp_prob_prime($n));
$n = gmp_init(0);
var_dump(gmp_prob_prime($n));

try { var_dump(gmp_prob_prime()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_prob_prime(vec[]));

echo "Done\n";
}

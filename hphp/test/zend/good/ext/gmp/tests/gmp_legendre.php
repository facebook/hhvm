<?php

for ($i = -1; $i < 10; $i++) {
	var_dump(gmp_strval(gmp_legendre(($i*$i)-1, 3)));
}

var_dump(gmp_strval(gmp_legendre(7, 23)));
var_dump(gmp_strval(gmp_legendre("733535124", "1234123423434535623")));
var_dump(gmp_strval(gmp_legendre(3, "1234123423434535623")));

$n = "123123";
$n1 = "1231231";

var_dump(gmp_strval(gmp_legendre($n, $n1)));
var_dump(gmp_strval(gmp_legendre($n, 3)));
var_dump(gmp_strval(gmp_legendre(3, $n1)));

var_dump(gmp_legendre(3, array()));
var_dump(gmp_legendre(array(), 3));
var_dump(gmp_legendre(array(), array()));

try { var_dump(gmp_legendre(array(), array(), 1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_legendre(array())); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_legendre()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
?>

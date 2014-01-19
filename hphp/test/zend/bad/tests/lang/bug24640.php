<?php
function test($v)
{
	echo var_export($v, true) . "\n";
	var_dump($v);
	echo "$v\n";
	print_r($v);
	echo "\n------\n";
}

test(1.7e+300);
test(1.7e-300);
test(1.7e+79);
test(1.7e-79);
test(1.7e+80);
test(1.7e-80);
test(1.7e+81);
test(1.7e-81);
test(1.7e+319);
test(1.7e-319);
test(1.7e+320);
test(1.7e-320);
test(1.7e+321);
test(1.7e-321);
test(1.7e+324);
test(1.7e-324);
test(1.7e+1000);
test(1.7e-1000);

?>
===DONE===
<?php exit(0); ?>
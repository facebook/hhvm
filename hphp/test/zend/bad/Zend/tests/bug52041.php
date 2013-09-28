<?php
function foo() {
	return $x;
}

foo()->a = 1;
foo()->a->b = 2;
foo()->a++;
foo()->a->b++;
foo()->a += 2;
foo()->a->b += 2;

foo()[0] = 1;
foo()[0][0] = 2;
foo()[0]++;
foo()[0][0]++;
foo()[0] += 2;
foo()[0][0] += 2;

var_dump(foo());
?>
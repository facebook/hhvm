<?php

class test {
	const val = 1;
}

function foo($v = test::val) {
	var_dump($v);
}

function bar($b = NoSuchClass::val) {
	var_dump($b);
}

foo();
foo(5);

bar(10);
bar();

echo "Done\n";
?>
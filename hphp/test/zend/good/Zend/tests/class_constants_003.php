<?php

$class_data = <<<DATA
<?php
class test {
	const val = 1;
}
?>
DATA;

$filename = dirname(__FILE__)."/cc003.dat";
file_put_contents($filename, $class_data);

function foo($v = test::val) {
	var_dump($v);
}

include $filename;

foo();
foo(5);

unlink($filename);

echo "Done\n";
?>

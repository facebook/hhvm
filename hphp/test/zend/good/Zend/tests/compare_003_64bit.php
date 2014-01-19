<?php

class test {
}

$a = array(
	array(1,2,3),
	"",
	1,
	2.5,
	0,
	"string",
	"123",
	"2.5",
	NULL,
	true,
	false,
	new stdclass,
	new stdclass,
	new test,
	array(),
	-PHP_INT_MAX-1,
	(string)(-PHP_INT_MAX-1),
);

$var_cnt = count($a);

function my_dump($var) {
	ob_start();
	var_dump($var);
	$buf = ob_get_clean();
	echo str_replace("\n", "", $buf);
}

foreach($a as $var) {
	for ($i = 0; $i < $var_cnt; $i++) {
		my_dump($var);
		echo ($var > $a[$i]) ? " > " : " <= ";
		my_dump($a[$i]);
		echo "\n";
	}
}	

echo "Done\n";
?>
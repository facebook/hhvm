<?php

$a = array(
	array(1,2,3),
	array("", "", ""),
	array(array(), 1, ""),
	array(1, array(), ""),
	array(1, "", array()),
	);

foreach ($a as $args) {
	var_dump(mb_ereg($args[0], $args[1], $args[2]));
	var_dump($args);
}
?>
===DONE===
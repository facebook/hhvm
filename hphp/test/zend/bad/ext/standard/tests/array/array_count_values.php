<?php
$arrays = array (
	array (),
	array (0),
	array (1),
	array (-1),
	array (0, 0),
	array (0, 1),
	array (1, 1),
	array (1, "hello", 1, "world", "hello"),
	array ("hello", "world", "hello"),
	array ("", "world", "", "hello", "world", "hello", "hello", "world", "hello"),
	array (0, array (1, "hello", 1, "world", "hello")),
	array (1, array (1, "hello", 1, "world", "hello"), array (1, "hello", 1, "world", "hello"), array (1, "hello", 1, "world", "hello")),
);

foreach ($arrays as $item) {
	var_dump (@array_count_values ($item));
	echo "\n";
}
?>
<?php

$array = range(1, 10);

preg_grep('/asdf/', $array);

while (list($x) = each($array)) {
	print $x;
}

?>
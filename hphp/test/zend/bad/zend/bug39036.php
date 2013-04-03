<?php

$key = 'asdf';

foreach (get_defined_vars() as $key => $value) {
	unset($$key);
}

var_dump($key);

echo "Done\n";
?>
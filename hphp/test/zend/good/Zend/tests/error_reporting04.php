<?php

error_reporting(E_ALL);

function foo() {
	echo $undef;
	error_reporting(E_ALL|E_STRICT);
}


foo(@$var);

var_dump(error_reporting());

echo "Done\n";
?>
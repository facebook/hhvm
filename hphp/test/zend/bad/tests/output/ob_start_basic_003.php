<?php

function f() {
	return "I have stolen your output";
}

ob_start('f');
cause_fatal_error(); // call undefined function
ob_end_flush();

echo "done (you shouldn't see this)";

?>
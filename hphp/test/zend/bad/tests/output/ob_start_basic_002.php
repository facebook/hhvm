<?php
function return_empty_string($string) {
	return "";
}

function return_false($string) {
	return false;
}

function return_null($string) {
	return null;
}

function return_string($string) {
	return "I stole your output.";
}

function return_zero($string) {
	return 0;
}

// Use each of the above functions as an output buffering callback:
$functions = get_defined_functions();
$callbacks = $functions['user'];
sort($callbacks);
foreach ($callbacks as $callback) {
  echo "--> Use callback '$callback':\n";
  ob_start($callback);
  echo 'My output.';
  ob_end_flush();
  echo "\n\n";
}

?>
==DONE==
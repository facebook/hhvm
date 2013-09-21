<?php
function callback($string) {
	static $callback_invocations;
	$callback_invocations++;
	return "[callback:$callback_invocations]$string\n";
}

ob_start('callback', 0, false);

echo "All of the following calls will fail to clean/remove the topmost buffer:\n";
var_dump(ob_clean());
var_dump(ob_end_clean());
var_dump(ob_end_flush());

echo "The OB nesting will still be 1 level deep:\n";
var_dump(ob_get_level());
?>
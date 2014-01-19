<?php
function callback($string) {
	static $callback_invocations;
	$callback_invocations++;
	return "[callback:$callback_invocations]$string\n";
}

ob_start('callback', 0, false);

echo "This call will obtain the content, but will not flush the buffer.";
$str = ob_get_flush();
var_dump($str);
?>
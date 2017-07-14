<?php
require_once __DIR__ . '/exceptions.inc';

try {
	echo 'TRY' . PHP_EOL;
	throw new Exception3;
} catch(Exception1 | Exception2 $e) {
	echo get_class($e) . PHP_EOL;
} catch(Exception3 | Exception4 $e) {
	echo get_class($e) . PHP_EOL;
} finally {
	echo 'FINALLY' . PHP_EOL;
}
?>

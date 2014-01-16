<?php
try {
	$a = new Phar('http://should.fail.com');
} catch (UnexpectedValueException $e) {
	echo $e->getMessage(),"\n";
}
try {
	$a = new Phar('http://');
} catch (UnexpectedValueException $e) {
	echo $e->getMessage(),"\n";
}
try {
	$a = new Phar('http:/');
} catch (UnexpectedValueException $e) {
	echo $e->getMessage(),"\n";
}
?>
===DONE===
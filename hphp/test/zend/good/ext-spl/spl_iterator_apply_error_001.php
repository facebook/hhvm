<?php

function test() {
	throw new Exception('Broken callback');
}

$it = new RecursiveArrayIterator(array(1, 21, 22));

try {
	iterator_apply($it, 'test');
} catch (Exception $e) {
	echo $e->getMessage();
}

?>
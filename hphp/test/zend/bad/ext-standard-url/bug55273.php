<?php
function test($s) {
	$v = chunk_split(base64_encode($s));
	$r = base64_decode($v, True);
	var_dump($v, $r);
}

test('PHP');
test('PH');
test('P');

?>
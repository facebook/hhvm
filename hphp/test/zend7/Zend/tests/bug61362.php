<?php
function test($arg){
	throw new Exception();
}

try {
	test('тест');
}
catch(Exception $e) {
	echo $e->getTraceAsString(), "\n";
	echo (string)$e;
}
?>

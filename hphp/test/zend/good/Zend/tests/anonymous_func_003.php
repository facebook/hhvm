<?php 

try {
	$a = create_function('', 'return new Exception("test");');
	throw $a();
} catch (Exception $e) {
	var_dump($e->getMessage() == 'test');
}

?>
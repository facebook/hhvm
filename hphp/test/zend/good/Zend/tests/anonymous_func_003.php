<?php 

try {
  $a = () ==> new Exception('test');
	throw $a();
} catch (Exception $e) {
	var_dump($e->getMessage() == 'test');
}


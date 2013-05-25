<?php

try {
	new SplFileObject('foo', array());
} catch (Exception $e) {
	var_dump($e->getMessage());
}

?>
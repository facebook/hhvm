<?php

try {
	try {
		try {
			try {
				throw new Exception(NULL);
			} catch (Exception $e) {
				var_dump($e->getMessage());
				throw $e;
			}
		} catch (Exception $e) {
			var_dump($e->getMessage());
			throw $e;
		}
	} catch (Exception $e) {
		var_dump($e->getMessage());
		throw $e;
	}
} catch (Exception $e) {
	var_dump($e->getMessage());
	throw $e;
}

?>
<?php

try {
	try {
		throw new Exception("First", 1, new Exception("Another", 0, NULL));
	}
	catch (Exception $e) {
		throw new Exception("Second", 2, $e);
	}
}
catch (Exception $e) {
	throw new Exception("Third", 3, $e);
}

?>
===DONE===
<?php
try {
	switch ("1" . (int)2) {
		case 12:
			throw new Exception();
	}
} catch (Exception $e) {
	echo "exception\n";
} 
?>

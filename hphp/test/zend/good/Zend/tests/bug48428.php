<?php
try {
		function x() { throw new Exception("ERROR"); }
				x(x());
} catch(Exception $e) {
		echo($e -> getMessage());
}
?>
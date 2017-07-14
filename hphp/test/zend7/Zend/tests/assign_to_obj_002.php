<?php

try {
	$this->a = new stdClass;
} catch (Error $e) { echo $e->getMessage(), "\n"; }

?>

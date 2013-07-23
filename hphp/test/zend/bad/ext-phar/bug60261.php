<?php

$nx = new Phar();
try {
	$nx->getLinkTarget();
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

?>
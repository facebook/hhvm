<?php
try {
	$a = new PDO("sqlite:/this/path/should/not/exist.db");
} catch (PDOException $e) {
	var_dump($e->getCode());
}
?>
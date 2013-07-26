<?php

try {
	phar::mount(1,1);
} catch (Exception $e) {
	var_dump($e->getMessage());
}

?>
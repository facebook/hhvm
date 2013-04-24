<?php

$diriter = new RecursiveIteratorIterator( new RecursiveDirectoryIterator('.') );

foreach ($diriter as $key => $file) {
	var_dump($file->getFilename());
	var_dump($file->getPath());
	break;
}

?>
===DONE===
<?php
$it = new DirectoryIterator(__DIR__); 
foreach( $it as &$file ) {
	echo $file . "\n";
}
?>
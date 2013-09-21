<?php
$fname = str_replace('\\', '/', dirname(__FILE__) . '/files/Structures_Graph-1.0.3.tgz');
$tar = new PharData($fname);
$files = array();
foreach (new RecursiveIteratorIterator($tar) as $file) {
	$files[] = str_replace($fname, '*', $file->getPathName());
}
print_r($files);
?>
===DONE===
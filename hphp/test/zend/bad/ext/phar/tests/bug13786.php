<?php

try {
for ($i = 0; $i < 2; $i++) {
	$fname = "DataArchive.phar";
	$path = dirname(__FILE__) . DIRECTORY_SEPARATOR . $fname;
	$phar = new Phar($path);
	$phar->addFromString($i, "file $i in $fname");
	var_dump(file_get_contents($phar[$i]));
	unset($phar);
	unlink($path);
}

echo("\nWritten files: $i\n");
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}

?>
===DONE===
<?php

$it = new RecursiveIteratorIterator(new RecursiveDirectoryIterator("."), true);

$idx = 0;
foreach($it as $file)
{
	echo "First\n";
	var_Dump($file->getFilename());
	echo "Second\n";
	var_dump($file->getFilename());
	if (++$idx > 1)
	{
		break;
	}
}

?>
===DONE===
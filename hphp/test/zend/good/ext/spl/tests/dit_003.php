<?php
$count = 0;
foreach(new FilesystemIterator(__DIR__) as $ent)
{
	++$count;
}
var_dump($count > 0);
?>
===DONE===

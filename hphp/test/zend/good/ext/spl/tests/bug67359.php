<?php
try
{
	$rdi = new recursiveDirectoryIterator(dirname(__FILE__),  FilesystemIterator::SKIP_DOTS | FilesystemIterator::UNIX_PATHS);
	$it = new recursiveIteratorIterator( $rdi );
	$it->seek(1);
	while( $it->valid())
	{
		if( $it->isFile() )
		{
			$it->current();
		}

		$it->next();
	}

	$it->current();
}
catch(Exception $e)
{
}
echo "okey"
?>

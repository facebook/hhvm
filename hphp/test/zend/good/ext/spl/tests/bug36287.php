<?hh
<<__EntryPoint>> function main(): void {
$it = new RecursiveIteratorIterator(new RecursiveDirectoryIterator("."), true);

$idx = 0;
foreach($it as $file)
{
	echo "First\n";
	var_dump($file->getFilename());
	echo "Second\n";
	var_dump($file->getFilename());
	if (++$idx > 1)
	{
		break;
	}
}

echo "===DONE===\n";
}

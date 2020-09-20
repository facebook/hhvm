<?hh
<<__EntryPoint>> function main(): void {
$diriter = new RecursiveIteratorIterator( new RecursiveDirectoryIterator('.') );

foreach ($diriter as $key => $file) {
	var_dump($file->getFilename());
	var_dump($file->getPath());
	break;
}

echo "===DONE===\n";
}

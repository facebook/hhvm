<?hh
<<__EntryPoint>> function main(): void {
$dirs = varray[];
$empty_dir = __SystemLib\hphp_test_tmppath('empty');
@mkdir($empty_dir);

$i = new RecursiveDirectoryIterator($empty_dir, FilesystemIterator::KEY_AS_PATHNAME | FilesystemIterator::CURRENT_AS_FILEINFO); // Note the absence of FilesystemIterator::SKIP_DOTS
foreach ($i as $key => $value) {
    $dirs[] = $value->getFilename();
}

@rmdir($empty_dir);

sort(inout $dirs);
print_r($dirs);
}

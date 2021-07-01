<?hh
<<__EntryPoint>> function main(): void {
$file = '__tmp14962.txt';
$fullpath = __SystemLib\hphp_test_tmppath($file);
$zipfile = __SystemLib\hphp_test_tmppath('__14962.zip');
$za = new ZipArchive;
$za->open($zipfile, ZipArchive::CREATE);
$za->addFromString($file, '1234');
$za->close();

if (!is_file($zipfile)) {
	die('failed to create the archive');
}
$za = new ZipArchive;
$za->open($zipfile);
$za->extractTo(__SystemLib\hphp_test_tmproot(), NULL);
$za->close();

if (is_file($fullpath)) {
	unlink($fullpath);
	echo "Ok";
}
unlink($zipfile);
}

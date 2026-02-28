<?hh
<<__EntryPoint>> function main(): void {
$file = '__tmp14962.txt';
$fullpath = sys_get_temp_dir().'/'.$file;
$zipfile = sys_get_temp_dir().'/'.'__14962.zip';
$za = new ZipArchive;
$za->open($zipfile, ZipArchive::CREATE);
$za->addFromString($file, '1234');
$za->close();

if (!is_file($zipfile)) {
	exit('failed to create the archive');
}
$za = new ZipArchive;
$za->open($zipfile);
$za->extractTo(sys_get_temp_dir(), NULL);
$za->close();

if (is_file($fullpath)) {
	unlink($fullpath);
	echo "Ok";
}
unlink($zipfile);
}

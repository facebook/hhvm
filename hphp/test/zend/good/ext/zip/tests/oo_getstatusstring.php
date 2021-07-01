<?hh
<<__EntryPoint>> function main(): void {
$dirname = dirname(__FILE__) . '/';
$arch = new ZipArchive;
$arch->open($dirname.'foo.zip',ZipArchive::CREATE);
var_dump($arch->getStatusString());
//delete an index that does not exist - trigger error
$arch->deleteIndex(2);
var_dump($arch->getStatusString());
$arch->close();
error_reporting(0);
unlink($dirname.'foo.zip');
}

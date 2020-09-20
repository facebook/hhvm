<?hh

<<__EntryPoint>>
function main_ziparchive_setcompression() {
$str = 'temp';
$dir = tempnam(sys_get_temp_dir(), __FILE__);
unlink($dir);
mkdir($dir);
$archive = new ZipArchive();
$archive->open("$dir/comptest.zip", ZipArchive::CREATE);
$archive->addFromString("A.txt", $str);
$archive->addFromString("B.txt", $str);
var_dump($archive->setCompressionIndex(0, ZIPArchive::CM_STORE));
var_dump($archive->setCompressionName("B.txt", ZIPArchive::CM_STORE));
$archive->close();
unlink("$dir/comptest.zip");
rmdir($dir);
}

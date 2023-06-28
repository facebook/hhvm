<?hh

<<__EntryPoint>>
function main_ziparchive_setcompression() :mixed{
$str = 'temp';
$dir = tempnam(sys_get_temp_dir(), __FILE__);
unlink($dir);
mkdir($dir);
$archive = new ZipArchive();
$archive->open("$dir/comptest.zip", ZipArchive::CREATE);
$archive->addFromString("A.txt", $str);
$archive->addFromString("B.txt", $str);
var_dump($archive->setCompressionIndex(0, ZipArchive::CM_STORE));
var_dump($archive->setCompressionName("B.txt", ZipArchive::CM_STORE));
$archive->close();
unlink("$dir/comptest.zip");
rmdir($dir);
}

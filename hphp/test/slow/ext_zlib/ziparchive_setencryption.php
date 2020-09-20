<?hh

<<__EntryPoint>>
function main_ziparchive_setencryption() {
$str = 'temp';
$archive = new ZipArchive();
$archive->open(__SystemLib\hphp_test_tmppath('comptest.zip'), ZipArchive::CREATE);
$archive->addFromString("A.txt", $str);
$archive->addFromString("B.txt", $str);
var_dump($archive->setEncryptionIndex(0, ZIPArchive::EM_AES_256, 'password'));
var_dump($archive->setEncryptionName("B.txt", ZIPArchive::EM_AES_256, 'password'));
$archive->close();
unlink(__SystemLib\hphp_test_tmppath('comptest.zip'));
}

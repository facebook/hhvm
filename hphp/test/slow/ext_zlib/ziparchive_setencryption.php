<?hh

<<__EntryPoint>>
function main_ziparchive_setencryption() :mixed{
$str = 'temp';
$archive = new ZipArchive();
$archive->open(sys_get_temp_dir().'/'.'comptest.zip', ZipArchive::CREATE);
$archive->addFromString("A.txt", $str);
$archive->addFromString("B.txt", $str);
var_dump($archive->setEncryptionIndex(0, ZipArchive::EM_AES_256, 'password'));
var_dump($archive->setEncryptionName("B.txt", ZipArchive::EM_AES_256, 'password'));
$archive->close();
unlink(sys_get_temp_dir().'/'.'comptest.zip');
}

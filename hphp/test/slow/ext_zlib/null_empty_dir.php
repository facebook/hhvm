<?hh


<<__EntryPoint>>
function main_null_empty_dir() :mixed{
$a = new ZipArchive();
var_dump($a->open('foo.zip', ZipArchive::CREATE));
var_dump($a->addEmptyDir(''));
var_dump($a->close());
}

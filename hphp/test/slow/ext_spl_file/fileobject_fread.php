<?hh


<<__EntryPoint>>
function main_fileobject_fread() :mixed{
$file = new SplFileObject(__FILE__);
var_dump($file->fread(4));
}

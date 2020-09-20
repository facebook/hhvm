<?hh


<<__EntryPoint>>
function main_fileobject_fread() {
$file = new SplFileObject(__FILE__);
var_dump($file->fread(4));
}

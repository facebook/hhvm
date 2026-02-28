<?hh


<<__EntryPoint>>
function main_spl_fileinfo_false() :mixed{
$fi = new SplFileInfo(false);
var_dump($fi->getPath());
}

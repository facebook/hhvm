<?hh


<<__EntryPoint>>
function main_spl_fileinfo_false() {
$fi = new SplFileInfo(false);
var_dump($fi->getPath());
}

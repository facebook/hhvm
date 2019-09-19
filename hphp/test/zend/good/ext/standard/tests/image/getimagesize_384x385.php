<?hh
<<__EntryPoint>> function main(): void {
// Note: SWC requires zlib
($dir = opendir(dirname(__FILE__))) || die('cannot open directory: '.dirname(__FILE__));
$result = array();
$files  = array();
while (($file = readdir($dir)) !== FALSE) {
    if (preg_match('/^384x385\./',$file)) {
        $files[] = $file;
    }
}
closedir($dir);
sort(&$files);
foreach($files as $file) {
    $result[$file] = getimagesize(dirname(__FILE__)."/$file");
}
var_dump($result);
}

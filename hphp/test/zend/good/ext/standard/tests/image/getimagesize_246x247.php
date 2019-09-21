<?hh
<<__EntryPoint>> function main(): void {
// Note: SWC requires zlib
($dir = opendir(dirname(__FILE__))) || die('cannot open directory: '.dirname(__FILE__));
$result = array();
$files  = array();
while (($file = readdir($dir)) !== FALSE) {
    if (preg_match('/^246x247\./',$file)) {
        $files[] = $file;
    }
}
closedir($dir);
sort(&$files);
$info = null;
foreach($files as $file) {
    $result[$file] = getimagesize(dirname(__FILE__)."/$file", inout $info);
}
var_dump($result);
}

<?hh
<<__EntryPoint>> function main(): void {
// Note: SWC requires zlib
($dir = opendir(dirname(__FILE__))) || die('cannot open directory: '.dirname(__FILE__));
$result = dict[];
$files  = vec[];
while (($file = readdir($dir)) !== FALSE) {
    if (preg_match('/^test.+pix\./',$file) && $file != "test13pix.swf") {
        $files[] = $file;
    }
}
closedir($dir);
sort(inout $files);
$info = null;
foreach($files as $file) {
    $result[$file] = getimagesize(dirname(__FILE__)."/$file", inout $info);
}
var_dump($result);
}

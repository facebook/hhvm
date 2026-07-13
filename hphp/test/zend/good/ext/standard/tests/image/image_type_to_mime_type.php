<?hh
<<__EntryPoint>> function main(): void {
// Note: SWC requires zlib
$dir = opendir(dirname(__FILE__));
$dir || exit('cannot open directory: '.dirname(__FILE__));
$result = dict[];
$files  = vec[];
$file = readdir($dir);
while ($file !== FALSE) {
    if (preg_match('/^test.+pix\./',$file) && $file != "test13pix.swf") {
        $files[] = $file;
    }
    $file = readdir($dir);
}
closedir($dir);
sort(inout $files);
$info = null;
foreach($files as $file) {
    $result[$file] = getimagesize(dirname(__FILE__)."/$file", inout $info);
    $result[$file] = image_type_to_mime_type($result[$file][2]);
}
var_dump($result);
}

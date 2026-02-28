<?hh
/* Prototype  : string readdir([resource $dir_handle])
 * Description: Read directory entry from dir_handle
 * Source code: ext/standard/dir.c
 */

/*
 * Pass readdir() a directory handle pointing to an empty directory to test behaviour
 */
function mysort($a,$b) :mixed{
    return strlen($a) > strlen($b) ? 1 : -1;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing readdir() : usage variations ***\n";

$path = sys_get_temp_dir().'/'.'readdir_variation2';
mkdir($path);
$dir_handle = opendir($path);

echo "\n-- Pass an empty directory to readdir() --\n";
$entries = vec[];
while(FALSE !== ($file = readdir($dir_handle))){
    $entries[] = $file;
}

closedir($dir_handle);

usort(inout $entries, mysort<>);
foreach($entries as $entry) {
    var_dump($entry);
}
echo "===DONE===\n";

rmdir($path);
}

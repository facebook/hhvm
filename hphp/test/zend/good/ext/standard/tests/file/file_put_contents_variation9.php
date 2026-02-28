<?hh
/* Prototype  : int file_put_contents(string file, mixed data [, int flags [, resource context]])
 * Description: Write/Create a file with contents data and return the number of bytes written
 * Source code: ext/standard/file.c
 * Alias to functions:
 */

function run_test($file) :mixed{
    $data = "Here is some data";
    $extra = ", more data";
    var_dump(file_put_contents($file, $data));
    var_dump(file_put_contents($file, $extra, FILE_APPEND));
    readfile($file);
    echo "\n";
}

<<__EntryPoint>> function main(): void {
echo "*** Testing file_put_contents() : usage variation ***\n";

$filename = sys_get_temp_dir().'/'.'filePutContentsVar9.tmp';
$softlink = sys_get_temp_dir().'/'.'filePutContentsVar9.SoftLink';
$hardlink = sys_get_temp_dir().'/'.'filePutContentsVar9.HardLink';
$chainlink = sys_get_temp_dir().'/'.'filePutContentsVar9.ChainLink';


// link files even though it original file doesn't exist yet
symlink($filename, $softlink);
symlink($softlink, $chainlink);


// perform tests
run_test($chainlink);
run_test($softlink);

//can only create a hardlink if the file exists.
file_put_contents($filename,"");
link($filename, $hardlink);
run_test($hardlink);

unlink($chainlink);
unlink($softlink);
unlink($hardlink);
unlink($filename);


echo "\n*** Done ***\n";
}

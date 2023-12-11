<?hh
/* Prototype  : array parse_ini_file(string filename [, bool process_sections])
 * Description: Parse configuration file
 * Source code: ext/standard/basic_functions.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing parse_ini_file() : variation ***\n";
chdir(sys_get_temp_dir());

$iniFile = "php.ini";
$newdirs = vec['dir1', 'dir2', 'dir3'];
$pathSep = ":";
$newIncludePath = "";
if(substr(PHP_OS, 0, 3) == 'WIN' ) {
   $pathSep = ";";
}
foreach($newdirs as $newdir) {
   mkdir($newdir);
   $newIncludePath .= $newdir.$pathSep;
}

set_include_path($newIncludePath);
$path = get_include_path();
echo "New include path is : " . $path . "\n";

$output_file = "dir2/".$iniFile;
$iniContent = <<<FILE
error_reporting  =  E_ALL
display_errors = On
display_startup_errors = Off
log_errors = Off
log_errors_max_len = 1024
ignore_repeated_errors = Off
ignore_repeated_source = Off
report_memleaks = On
track_errors = Off
docref_root = "/phpmanual/"
docref_ext = .html

FILE;

file_put_contents($output_file, $iniContent);
var_dump(parse_ini_file($iniFile));

echo "===Done===\n";

unlink($output_file);
foreach($newdirs as $newdir) {
   rmdir($newdir);
}
}

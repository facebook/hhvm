<?hh
/*  Prototype: bool mkdir ( string $pathname [, int $mode [, bool $recursive [, resource $context]]] );
    Description: Makes directory
*/
<<__EntryPoint>> function main(): void {
$context = stream_context_create();

$tmpdir = sys_get_temp_dir();

echo "\n*** Testing mkdir() and rmdir() by giving stream context as fourth argument ***\n";
var_dump( mkdir("$tmpdir/mkdir_variation2/test/", 0777, true, $context) );
var_dump( rmdir("$tmpdir/mkdir_variation2/test/", $context) );

echo "\n*** Testing rmdir() on a non-empty directory ***\n";
var_dump( mkdir("$tmpdir/mkdir_variation2/test/", 0777, true) );
var_dump( rmdir("$tmpdir/mkdir_variation2/") );

echo "\n*** Testing mkdir() and rmdir() for binary safe functionality ***\n";
var_dump( mkdir("$tmpdir/temp".chr(0)."/") );
var_dump( rmdir("$tmpdir/temp".chr(0)."/") );

echo "\n*** Testing mkdir() with miscelleneous input ***\n";
/* changing mode of mkdir to prevent creating sub-directory under it */
var_dump( chmod("$tmpdir/mkdir_variation2/", 0000) );
/* creating sub-directory test1 under mkdir, expected: false */
var_dump( mkdir("$tmpdir/mkdir_variation2/test1", 0777, true) );
var_dump( chmod("$tmpdir/mkdir_variation2/", 0777) );  // chmod to enable removing test1 directory

echo "Done\n";

rmdir("$tmpdir/mkdir_variation2/test/");
rmdir("$tmpdir/mkdir_variation2/");
}

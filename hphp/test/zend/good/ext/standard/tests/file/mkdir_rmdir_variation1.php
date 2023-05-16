<?hh
/*  Prototype: bool mkdir ( string $pathname [, int $mode [, bool $recursive [, resource $context]]] );
    Description: Makes directory
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing mkdir() and rmdir() for different permissions ***\n";

$context = stream_context_create();

$tmpdir = sys_get_temp_dir();
$counter = 1;

for($mode = 0000; $mode <= 0777; $mode++) {
  echo "-- Changing mode of directory to $mode --\n";
  var_dump( mkdir("$tmpdir/mkdir_variation1/", $mode, true) );
  var_dump( rmdir("$tmpdir/mkdir_variation1/") );
  $counter++;
}

echo "Done\n";
}

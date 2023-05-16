<?hh
/* Prototype  : resource gzopen(string filename, string mode [, int use_include_path])
 * Description: Open a .gz-file and return a .gz-file pointer 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing gzopen() : variation ***\n";

$data = <<<EOT
Here is some plain
text to be read
and displayed.
EOT;

$file = sys_get_temp_dir().'/'."gzopen_variation8.tmp";
$h = fopen($file, 'w');
fwrite($h, $data);
fclose($h);

$h = gzopen($file, 'r');
gzpassthru($h);
gzclose($h);
echo "\n";
unlink($file);
echo "===DONE===\n";
}

<?hh
/* Prototype  : array file(string filename [, int flags[, resource context]])
 * Description: Read entire file into an array
 * Source code: ext/standard/file.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing file() : variation ***\n";
$testfile = sys_get_temp_dir().'/'.'fileVar9.txt';

$contents = vec[
   "File ends on a single character\na",
   "File ends on a new line\n",
   "File ends on multiple newlines\n\n\n\n",
   "File has\n\nmultiple lines and newlines\n\n",
   "File has\r\nmultiple crlfs\n\r\n"
   ];

@unlink($testfile);
foreach ($contents as $content) {
    $h = fopen($testfile, "w");
    fwrite($h, $content);
    fclose($h);
    var_dump(file($testfile));
    unlink($testfile);
}

echo "\n*** Done ***\n";
}

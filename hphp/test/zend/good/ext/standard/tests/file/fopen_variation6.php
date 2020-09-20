<?hh
/* Prototype  : resource fopen(string filename, string mode [, bool use_include_path [, resource context]])
 * Description: Open a file or a URL and return a file pointer 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing fopen() : variation ***\n";
chdir(__SystemLib\hphp_test_tmproot());
$absfile = __SystemLib\hphp_test_tmppath('fopen_variation6.php.tmp');
$relfile = "fopen_variation6.tmp";

$h = fopen($absfile, "w");
fwrite($h, "This is an absolute file");
fclose($h);

$h = fopen($relfile, "w");
fwrite($h, "This is a relative file");
fclose($h);

$ctx = stream_context_create();
$h = fopen($absfile, "r", true, $ctx);
fpassthru($h);
fclose($h);
echo "\n";

$h = fopen($relfile, "r", true, $ctx);
fpassthru($h);
fclose($h);
echo "\n";

unlink($absfile);
unlink($relfile);
echo "===DONE===\n";
}

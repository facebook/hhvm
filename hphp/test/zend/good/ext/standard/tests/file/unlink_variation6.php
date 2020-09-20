<?hh
/* Prototype  : bool unlink(string filename[, context context])
 * Description: Delete a file
 * Source code: ext/standard/file.c
 * Alias to functions:
 */
function f_exists($file) {
   if (file_exists($file) == true) {
      echo "$file exists\n";
   }
   else {
      echo "$file doesn't exist\n";
   }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing unlink() : variation: contexts and relative files ***\n";
chdir(__SystemLib\hphp_test_tmproot());
// test relative directories and stream contexts.
$subdir = 'subdir';
$testfile = $subdir.'/testfile.txt';
mkdir($subdir);
touch($testfile);
f_exists($testfile);
$context = stream_context_create();
var_dump(unlink($testfile, $context));
f_exists($testfile);
rmdir($subdir);

echo "===DONE===\n";
}

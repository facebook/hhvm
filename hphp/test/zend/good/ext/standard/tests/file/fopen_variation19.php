<?hh
/* Prototype  : resource fopen(string filename, string mode [, bool use_include_path [, resource context]])
 * Description: Open a file or a URL and return a file pointer
 * Source code: ext/standard/file.c
 * Alias to functions:
 */

function readFile2($file) :mixed{
   $h = fopen($file, 'r');
   fpassthru($h);
   fclose($h);
   echo "\n";
}

function appendFile($file) :mixed{
   $h = fopen($file, 'a+');
   fwrite($h, ' again!');
   fseek($h, 0);
   fpassthru($h);
   fclose($h);
   echo "\n";
}

function writeFile($file) :mixed{
   $h = fopen($file, 'w');
   fwrite($h, 'Goodbye World');
   fclose($h);
   readFile2($file);
}
<<__EntryPoint>> function main(): void {
$tmpDir = sys_get_temp_dir().'/'.'fopenVar19.Dir';
$realFilename = basename(__FILE__).'.real';
$sortFilename = basename(__FILE__).'.soft';
$hardFilename = basename(__FILE__).'.hard';
$linkOfLink = basename(__FILE__).'.soft2';

echo "*** Testing fopen() : variation ***\n";
// start the test
mkdir($tmpDir);
$oldDirPath = getcwd();
chdir($tmpDir);

$h = fopen($realFilename, "w");
fwrite($h, "Hello World");
fclose($h);

symlink($realFilename, $sortFilename);
symlink($sortFilename, $linkOfLink);
link($realFilename, $hardFilename);



echo "*** testing reading of links ***\n";
echo "soft link:";
readFile2($sortFilename);
echo "hard link:";
readFile2($hardFilename);
echo "link of link:";
readFile2($linkOfLink);

echo "*** test appending to links ***\n";
echo "soft link:";
appendFile($sortFilename);
echo "hard link:";
appendFile($hardFilename);
echo "link of link:";
appendFile($linkOfLink);

echo "*** test overwriting links ***\n";
echo "soft link:";
writeFile($sortFilename);
echo "hard link:";
writeFile($hardFilename);
echo "link of link:";
writeFile($linkOfLink);

unlink($linkOfLink);
unlink($sortFilename);
unlink($hardFilename);
unlink($realFilename);
chdir($oldDirPath);
rmdir($tmpDir);

echo "===DONE===\n";
}

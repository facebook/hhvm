<?php
/* Prototype  : string file_get_contents(string filename [, bool use_include_path [, resource context [, long offset [, long maxlen]]]])
 * Description: Read the entire file into a string 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing file_get_contents() : variation ***\n";
$filename = dirname(__FILE__).'/fileGetContentsVar9.tmp';
$softlink = dirname(__FILE__).'/fileGetContentsVar9.SoftLink';
$hardlink = dirname(__FILE__).'/fileGetContentsVar9.HardLink';
$chainlink = dirname(__FILE__).'/fileGetContentsVar9.ChainLink';

// create file
$h = fopen($filename,"w");
//Data should be more than the size of a link.
for ($i = 1; $i <= 10; $i++) {
   fwrite($h, b"Here is a repeated amount of data");
}
fclose($h);

// link files
link($filename, $hardlink);
symlink($filename, $softlink);
symlink($softlink, $chainlink);

// perform tests
var_dump(file_get_contents($chainlink));
var_dump(file_get_contents($softlink));
var_dump(file_get_contents($hardlink));

unlink($chainlink);
unlink($softlink);
unlink($hardlink);
unlink($filename);

echo "\n*** Done ***\n";
?>

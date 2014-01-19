<?php
/* Prototype  : resource gzopen(string filename, string mode [, int use_include_path])
 * Description: Open a .gz-file and return a .gz-file pointer 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

echo "*** Testing gzopen() : variation ***\n";
$absfile = __FILE__.'.tmp';
$relfile = "gzopen_variation6.tmp";

$h = gzopen($absfile, "w");
gzwrite($h, "This is an absolute file");
gzclose($h);

$h = gzopen($relfile, "w");
gzwrite($h, "This is a relative file");
gzclose($h);

$h = gzopen($absfile, "r");
gzpassthru($h);
fclose($h);
echo "\n";

$h = gzopen($relfile, "r");
gzpassthru($h);
gzclose($h);
echo "\n";

unlink($absfile);
unlink($relfile);
?>
===DONE===
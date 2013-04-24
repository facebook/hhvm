<?php
/* Prototype  : proto int fseek(resource fp, int offset [, int whence])
 * Description: Seek on a file pointer 
 * Source code: ext/standard/file.c
 * Alias to functions: gzseek
 */

echo "*** Testing fseek() : variation - beyond file boundaries ***\n";

$outputfile = __FILE__.".tmp";

$h = fopen($outputfile, "wb+");
for ($i = 1; $i < 10; $i++) {
   fwrite($h, chr(0x30 + $i));
}

echo "--- fseek beyond start of file ---\n";
var_dump(fseek($h, -4, SEEK_SET));
echo "after -4 seek: ".bin2hex(fread($h,1))."\n";
var_dump(fseek($h, -1, SEEK_CUR));
echo "after seek back 1: ".bin2hex(fread($h,1))."\n";
var_dump(fseek($h, -20, SEEK_CUR));
echo "after seek back 20: ".bin2hex(fread($h,1))."\n";

echo "--- fseek beyond end of file ---\n";
var_dump(fseek($h, 16, SEEK_SET));
fwrite($h, b"end");
fseek($h ,0, SEEK_SET);
$data = fread($h, 4096);
echo bin2hex($data)."\n";

fclose($h);
unlink($outputfile);

echo "Done";
?>
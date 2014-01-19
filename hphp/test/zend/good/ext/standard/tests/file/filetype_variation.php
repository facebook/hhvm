<?php
/*
Prototype: string filetype ( string $filename );
Description: Returns the type of the file. Possible values are fifo, char,
             dir, block, link, file, and unknown. 
*/

echo "*** Testing filetype() with various types ***\n";
$file_path = dirname(__FILE__);
$file1 = $file_path."/filetype1_variation.tmp";
$file2 = $file_path."/filetype2_variation.tmp";
$file3 = $file_path."/filetype3_variation.tmp";
$link1 = $file_path."/filetype1_variation_link.tmp";
$link2 = $file_path."/filetype2_variation_link.tmp";

fclose( fopen($file1, "w") );
fclose( fopen($file2, "w") );

echo "-- Checking with files --\n";
print( filetype($file1) )."\n";
print( filetype($file2) )."\n";
clearstatcache();

echo "-- Checking with links: hardlink --\n";
link( $file1, $link1);
print( filetype($link1 ) )."\n";

echo "-- Checking with links: symlink --\n";
symlink( $file2, $link2);
print( filetype($link2) )."\n";

unlink($link1);
unlink($link2);
unlink($file1);
unlink($file2);

echo "-- Checking with directory --\n";
mkdir("$file_path/filetype_variation");
print( filetype("$file_path/filetype_variation") )."\n";
rmdir( "$file_path/filetype_variation" );

echo "-- Checking with fifo --\n";
posix_mkfifo( $file3, 0755);
print( filetype( $file3) )."\n";
unlink($file3);

/* Checking with block in file */
/* To test this PEAR package should be installed */

echo "\n*** Done ***\n";
?>
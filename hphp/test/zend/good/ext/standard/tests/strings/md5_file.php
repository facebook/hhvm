<?php

/* Prototype: string md5_file( string filename[, bool raw_output] )
 * Description: Calculate the MD5 hash of a given file
 */

/* Creating an empty file */
if (($handle = fopen( "md5_EmptyFile.txt", "w+")) == FALSE)
return false;

/* Creating a data file */
if (($handle2 = fopen( "md5_DataFile.txt", "w+")) == FALSE)
return false;

/* Writing into file */ 
$filename = "md5_DataFile.txt";
$content = "Add this to the file\n";
if (is_writable($filename)) {
  if (fwrite($handle2, $content) === FALSE) {
    echo "Cannot write to file ($filename)";
    exit;
  }
}

// close the files 
fclose($handle);
fclose($handle2);

/* Testing error conditions */
echo "\n*** Testing for error conditions ***\n";

/* No filename */
var_dump( md5_file("") );

/* invalid filename */
var_dump( md5_file("aZrq16u") );

/* Scalar value as filename  */
var_dump( md5_file(12) );

/* NULL as filename */
var_dump( md5_file(NULL) );

/* Zero arguments */
 try { var_dump ( md5_file() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* More than valid number of arguments ( valid is 2)  */
try { var_dump ( md5_file("md5_EmptyFile.txt", true, NULL) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Hexadecimal Output for Empty file as input */
echo "\n*** Hexadecimal Output for Empty file as Argument ***\n";
var_dump( md5_file("md5_EmptyFile.txt") );

/* Raw Binary Output for Empty file as input */
echo "\n*** Raw Binary Output for Empty file as Argument ***\n";
var_dump( md5_file("md5_EmptyFile.txt", true) );

/* Normal operation with hexadecimal output */
echo "\n*** Hexadecimal Output for a valid file with some contents ***\n";
var_dump( md5_file("md5_DataFile.txt") );

/* Normal operation with raw binary output */
echo "\n*** Raw Binary Output for a valid file with some contents ***\n";
var_dump ( md5_file("md5_DataFile.txt", true) );

// remove temp files
unlink("md5_DataFile.txt");
unlink("md5_EmptyFile.txt");

echo "\nDone";
?>

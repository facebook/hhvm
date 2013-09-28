<?php

/* Prototype: string md5_file( string filename[, bool raw_output] )
 * Description: Calculate the MD5 hash of a given file
 */

/* Creating an empty file */
if (($handle = fopen( "EmptyFile.txt", "w+")) == FALSE)
return false;

/* Creating a data file */
if (($handle2 = fopen( "DataFile.txt", "w+")) == FALSE)
return false;

/* Writing into file */ 
$filename = "DataFile.txt";
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
 var_dump ( md5_file() );

/* More than valid number of arguments ( valid is 2)  */
var_dump ( md5_file("EmptyFile.txt", true, NULL) );

/* Hexadecimal Output for Empty file as input */
echo "\n*** Hexadecimal Output for Empty file as Argument ***\n";
var_dump( md5_file("EmptyFile.txt") );

/* Raw Binary Output for Empty file as input */
echo "\n*** Raw Binary Output for Empty file as Argument ***\n";
var_dump( md5_file("EmptyFile.txt", true) );

/* Normal operation with hexadecimal output */
echo "\n*** Hexadecimal Output for a valid file with some contents ***\n";
var_dump( md5_file("DataFile.txt") );

/* Normal operation with raw binary output */
echo "\n*** Raw Binary Output for a valid file with some contents ***\n";
var_dump ( md5_file("DataFile.txt", true) );

// remove temp files
unlink("DataFile.txt");
unlink("EmptyFile.txt");

echo "\nDone";
?>
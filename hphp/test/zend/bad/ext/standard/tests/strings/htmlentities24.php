<?php
/* Prototype: string htmlentities ( string $string [, int $quote_style [, string $charset]] );
   Description: Convert all applicable characters to HTML entities
*/

/* retrieving htmlentities from the ANSI character table */
echo "*** Retrieving htmlentities for 256 characters ***\n";
for($i=0; $i<256; $i++)
  var_dump( bin2hex( htmlentities(b"chr($i)")) );

/* giving arguments as NULL */
echo "\n*** Testing htmlentities() with NULL as first,second and third argument ***\n";
var_dump( htmlentities("\x82\x86\x99\x9f\x80\x82\x81", NULL, 'cp1252') );
var_dump( htmlentities("\x82\x86\x99\x9f\x80\x82\x81", ENT_QUOTES, NULL) ); /* UTF-8 assumed */
var_dump( htmlentities("\x82\x86\x99\x9f\x80\x82\x81", ENT_NOQUOTES, NULL) ); /* UTF-8 assumed */
var_dump( htmlentities("\x82\x86\x99\x9f\x80\x82\x81", ENT_COMPAT, NULL) ); /* UTF-8 assumed */
var_dump( htmlentities(NULL, NULL, NULL) );

/* giving long string to check for proper memory re-allocation */
echo "\n*** Checking for proper memory allocation with long string ***\n";
var_dump( htmlentities("\x82\x86\x99\x9f\x80\x82\x86\x84\x80\x89\x85\x83\x86\x84\x80\x91\x83\x91\x86\x87\x85\x86\x88\x82\x89\x92\x91\x83", ENT_QUOTES, 'cp1252'));

/* giving a normal string */
echo "\n*** Testing a normal string with htmlentities() ***\n";
var_dump( htmlentities("<html> This is a test! </html>") );

/* checking behavior of quote */
echo "\n*** Testing htmlentites() on a quote ***\n";
$str = "A 'quote' is <b>bold</b>";
var_dump( htmlentities($str) );
var_dump( htmlentities($str, ENT_QUOTES) );
var_dump( htmlentities($str, ENT_NOQUOTES) );
var_dump( htmlentities($str, ENT_COMPAT) );

echo "\n*** Testing error conditions ***\n";
/* zero argument */
var_dump( htmlentities() );
/* arguments more than expected */
var_dump( htmlentities("\x84\x91",ENT_QUOTES, 'cp1252', "test1") );

echo "Done\n";
?>

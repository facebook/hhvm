<?php
/* Prototype  : proto int strcspn(string str, string mask [, int start [, int len]])
 * Description: Finds length of initial segment consisting entirely of characters not found in mask.
                If start or/and length is provided works like strcspn(substr($s,$start,$len),$bad_chars) 
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* Testing strcspn() : with varying mask and default start and len arguments
*/

echo "*** Testing strcspn() : with different mask strings and default start and len arguments ***\n";

// initialing required variables
$strings = array(
                   "",
		   '',
		   "\n",
		   '\n',
		   "hello\tworld\nhello\nworld\n",
		   'hello\tworld\nhello\nworld\n',
 		   "1234hello45world\t123",
 		   '1234hello45world\t123',
		   "hello\0world\012",
		   'hello\0world\012',
		   chr(0).chr(0),
		   chr(0)."hello\0world".chr(0),
		   chr(0).'hello\0world'.chr(0),
		   "hello".chr(0)."world",
		   'hello'.chr(0).'world',
		   "hello\0\100\xaaaworld",
		   'hello\0\100\xaaaworld'
                   );

// defining array of mask strings
$mask_array = array(
		    "",
		    '',
		    "\n\trsti \l",
		    '\n\trsti \l',
		    "\t",
		    "t\ ",
		    '\t',
		    "\t\ ",
		    " \t",
                    "\t\i\100\xa"
                   );
		

// loop through each element of the array for mask argument
$count = 1;
foreach($strings as $str) {
  echo "\n-- Itearation $count --\n";
  foreach($mask_array as $mask) {
      var_dump( strcspn($str,$mask) );
  }
  $count++;
}

echo "Done"
?>
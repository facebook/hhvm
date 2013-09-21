<?php
/* Prototype  : proto int strspn(string str, string mask [, int start [, int len]])
 * Description: Finds length of initial segment consisting entirely of characters found in mask.
                If start or/and length is provided works like strspn(substr($s,$start,$len),$good_chars) 
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* Testing strspn() : with varying mask and default start and len arguments
*/

echo "*** Testing strspn() : with different mask strings and default start and len arguments ***\n";

// initialing required variables
// defining different strings
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

// define the array of mask strings
$mask_array = array(
		    "",
		    '',
		    "f\n\trelshti \l",
		    'f\n\trelsthi \l',
		    "\telh",
		    "t\ ",
		    '\telh',
		    "felh\t\ ",
		    " \t",
                    "fhel\t\i\100\xa"
                   );
		

// loop through each element of the array for mask argument

$count = 1;
foreach($strings as $str)  {
  echo "\n-- Iteration $count --\n";
  foreach($mask_array as $mask)  {
      var_dump( strspn($str,$mask) );
  }
  $count++;
}

echo "Done"
?>
<?php
/* Prototype  : bool ctype_cntrl(mixed $c)
 * Description: Checks for control character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass strings containing different character types to ctype_cntrl() to test
 * which are considered valid control character only strings
 */

echo "*** Testing ctype_cntrl() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

$values = array(
/*1*/  "This string contains just letters and spaces", // Simple string
       "but this one contains some numbers too 123+456 = 678", // Mixed string
       "",
       " ",
/*5*/  "a",
       "ABCXYZ",
       "abcxyz",
       "ABCXYZ123DEF456",
       "abczyz123DEF456",
/*10*/ "\r\n",
       "123",
       "03F", // hexadecimal 'digits'
       ")speci@! ch@r$(",
       '@!$*',
/*15*/ 'ABC',
       'abc',
       'ABC123',
       'abc123',
       'abc123\n',
/*20*/ 'abc 123',
       '',
       ' ', 
       base64_decode("w4DDoMOHw6fDiMOo"), // non-ascii characters
       "\n",
/*25*/ "\r",
       "\t",
       "0xD",
       "0xA",
       "0xE",
/*30*/ "\t\r\n",
);


// loop through each element of $values to test behaviour of ctype_cntrl()
$iterator = 1;
foreach($values as $value) {
      echo "\n-- Iteration $iterator --\n";
      var_dump( ctype_cntrl($value) );
      $iterator++;
};

setlocale(LC_CTYPE, $orig);
?>
===DONE===
<?php
/* Prototype  : bool ctype_space(mixed $c)
 * Description: Checks for whitespace character(s)
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass strings containing different character types to ctype_space() to test
 * which are considered valid whitespace character only strings
 */

echo "*** Testing ctype_space() : usage variations ***\n";

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
       "abc123\n",
/*20*/ 'abc 123',
       '',
       ' ', 
       base64_decode("w4DDoMOHw6fDiMOo"), // non-ascii characters
       "\"\n",
/*25*/ " \t\r\n",
/*26*/ "\v\f",
);

$iterator = 1;
foreach($values as $value) {
      echo "\n-- Iteration $iterator --\n";
      var_dump( ctype_space($value) );
      $iterator++;
};

setlocale(LC_CTYPE, $orig);
?>
===DONE===
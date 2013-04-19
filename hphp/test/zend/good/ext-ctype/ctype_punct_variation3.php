<?php
/* Prototype  : bool ctype_punct(mixed $c)
 * Description: Checks for any printable character which is not whitespace 
 * or an alphanumeric character 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass strings containing different character types to ctype_punct() to test
 * which are considered valid punctuation character only strings
 */

echo "*** Testing ctype_punct() : usage variations ***\n";

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
       "!$%@\n",
/*25*/ "\"?!\"",
/*26*/ "|\ @~",
);

$iterator = 1;
foreach($values as $value) {
      echo "\n-- Iteration $iterator --\n";
      var_dump( ctype_punct($value) );
      $iterator++;
};
setlocale(LC_CTYPE, $orig);
?>
===DONE===
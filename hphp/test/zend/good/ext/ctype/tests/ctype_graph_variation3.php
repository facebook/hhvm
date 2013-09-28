<?php
/* Prototype  : bool ctype_graph(mixed $c)
 * Description: Checks for any printable character(s) except space 
 * Source code: ext/ctype/ctype.c 
 */

/*
 * Pass strings containing different character types to ctype_graph() to test
 * which are considered valid printable character only strings
 */

echo "*** Testing ctype_graph() : usage variations ***\n";

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
       "!$%^&*()_+-={}[]:;@~'#<,>.?/",
/*25*/ "\"ABC\"",
       "String\twith\ttabs",
       "Sample string with newline\n",
/*28*/ "123 ABC XYZ",
);

$iterator = 1;
foreach($values as $value) {
      echo "\n-- Iteration $iterator --\n";
      var_dump( ctype_graph($value) );
      $iterator++;
};

setlocale(LC_CTYPE, $orig);
?>
===DONE===
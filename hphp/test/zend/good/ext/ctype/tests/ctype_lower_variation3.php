<?hh
/* Prototype  : bool ctype_lower(mixed $c)
 * Description: Checks for lowercase character(s)  
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass strings containing different character types to ctype_lower() to test
 * which are considered valid lowercase character only strings
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_lower() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

$values = vec[
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
       "abcXYZ",
/*25*/ "abc xyz",
/*26*/ "abc+efg*xyz",
];

$iterator = 1;
foreach($values as $value) {
      echo "\n-- Iteration $iterator --\n";
      var_dump( ctype_lower($value) );
      $iterator++;
};

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}

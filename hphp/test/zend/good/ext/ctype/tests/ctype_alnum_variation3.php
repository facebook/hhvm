<?hh
/* Prototype  : bool ctype_alnum(mixed $c)
 * Description: Checks for alphanumeric character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different strings to ctype_alnum to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_alnum() : usage variations ***\n";

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
/*23*/ base64_decode("w4DDoMOHw6fDiMOo") // non-ascii characters
];


// loop through each element of $values to test behaviour of ctype_alnum()
$iterator = 1;
foreach($values as $value) {
      echo "\n-- Iteration $iterator --\n";
      var_dump( ctype_alnum($value) );
      $iterator++;
};

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}

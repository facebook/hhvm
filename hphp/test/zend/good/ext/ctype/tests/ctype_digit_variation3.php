<?hh
/* Prototype  : bool ctype_digit(mixed $c)
 * Description: Checks for numeric character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass strings containing different character types to ctype_digit() to test
 * which are considered valid decimal digit only strings
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_digit() : usage variations ***\n";

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
       ")speci@! ch@r$(",
       '@!$*',
       "0",
/*15*/ "3",
       "9", 
       "1234",
       "7890",
       "0677",
/*20*/ '0',
       '3',
       '9',
       '1234',
       '7890',
/*25*/ "123abc",
       "abc123",
       "123\r\t",
       "123 ",
       "  123",
/*30*/ "123E4",
/*31*/ "0x3F",
];

$iterator = 1;
foreach($values as $value) {
      echo "\n-- Iteration $iterator --\n";
      var_dump( ctype_digit($value) );
      $iterator++;
};

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}

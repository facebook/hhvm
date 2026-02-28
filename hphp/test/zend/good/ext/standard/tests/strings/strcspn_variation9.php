<?hh
/* Prototype  : proto int strcspn(string str, string mask [, int start [, int len]])
 * Description: Finds length of initial segment consisting entirely of characters not found in mask.
                If start or/and length is provided works like strcspn(substr($s,$start,$len),$bad_chars)
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* Testing strcspn() : with different strings as str argument and default start and len args
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strcspn() : with different str and default start and len args ***\n";

// initialing required variables
// defining different strings

$strings = vec[
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
                   ];

$mask = "sft\n34lw56r78d90\0\xaa\100o";


// loop through each element of the array for str argument

foreach($strings as $str) {
      echo "\n-- Iteration with str value \"$str\" --\n";

      //calling strcspn() with default arguments
      var_dump( strcspn($str,$mask) );
};

echo "Done";
}

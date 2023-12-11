<?hh
/* Prototype  : string strrchr(string $haystack, string $needle);
 * Description: Finds the last occurrence of a character in a string.
 * Source code: ext/standard/string.c
*/

/* Test strrchr() function: with binary values & null terminated strings passed to 'str1' & 'str2' */
<<__EntryPoint>> function main(): void {
echo "*** Test strrchr() function: binary safe ***\n";
$haystacks = vec[
  "Hello".chr(0)."World",
  chr(0)."Hello World",
  "Hello World".chr(0),
  chr(0).chr(0).chr(0),
  "Hello\0world",
  "\0Hello",
  "Hello\0"
];

for($index = 0; $index < count($haystacks); $index++ ) {
  //needle as null string
  var_dump( strrchr($haystacks[$index], "\0") );
  //needle as NULL
  var_dump( strrchr($haystacks[$index], NULL) );
}
echo "*** Done ***";
}

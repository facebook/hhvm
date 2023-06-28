<?hh
/* Prototype  : int strnatcasecmp(string s1, string s2)
 * Description: Returns the result of case-insensitive string comparison using 'natural' algorithm
 * Source code: ext/standard/string.c
 * Alias to functions:
 */

function str_dump($a, $b) :mixed{
    var_dump(strnatcasecmp($a, $b));
}
<<__EntryPoint>> function main(): void {
echo "*** Testing strnatcasecmp() : variation ***\n";

str_dump('0', '');
str_dump('fooBar', '');
str_dump("Hello\0world", "Helloworld");
str_dump("\x0", "\0");

echo "===DONE===\n";
}

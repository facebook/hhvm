<?hh
/* Prototype  : int strnatcasecmp(string s1, string s2)
 * Description: Returns the result of case-insensitive string comparison using 'natural' algorithm
 * Source code: ext/standard/string.c
 * Alias to functions:
 */

function str_dump($one, $two) {
    var_dump(strnatcasecmp($one, $two));
}
<<__EntryPoint>> function main(): void {
echo "*** Testing strnatcasecmp() : basic functionality ***\n";

// Calling strnatcasecmp() with all possible arguments
str_dump('A', 'a');
str_dump('a10', 'A20');
str_dump('A1b', 'a');
str_dump('x2-y7', 'x8-y8');
str_dump('1.010', '1.001');
str_dump(' ab', ' aB');
str_dump('acc ', 'acc');
str_dump('11.5', '10.5');
str_dump((string)10.5, (string)10.5E1);
str_dump('Rfc822.txt', 'rfc2086.txt');
str_dump('Rfc822.txt', 'rfc822.TXT');
str_dump('pIc 6', 'pic   7');
str_dump((string)0xFFF, (string)0Xfff);

echo "===DONE===\n";
}

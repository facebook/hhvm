<?hh
/* Prototype  : string mb_decode_mimeheader(string string)
 * Description: Decodes the MIME "encoded-word" in the string 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_decode_mimeheader() : basic functionality ***\n";
mb_internal_encoding('utf-8');

//the following encoded-words are identical and are UTF-8 Japanese. 
$a = "=?UTF-8?b?5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CC?=";
$b = mb_decode_mimeheader($a);
var_dump(bin2hex($b));

$a = "=?UTF-8?Q?=E6=97=A5=E6=9C=AC=E8=AA=9E=E3=83=86=E3=82=AD=E3=82=B9=E3=83=88?=
=?UTF-8?Q?=E3=81=A7=E3=81=99=E3=80=82?=";
$b = mb_decode_mimeheader($a);
var_dump(bin2hex($b));

echo "===DONE===\n";
}

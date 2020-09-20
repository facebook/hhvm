<?hh
/* Prototype  : void parse_str  ( string $str  [, array &$arr  ] )
 * Description: Parses the string into variables
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "\nTest string with badly formed strings\n";
$str = "arr[1=sid&arr[4][2=fred";
$res = null;
var_dump(parse_str($str, inout $res));
var_dump($res);

$str = "arr1]=sid&arr[4]2]=fred";
var_dump(parse_str($str, inout $res));
var_dump($res);

$str = "arr[one=sid&arr[4][two=fred";
var_dump(parse_str($str, inout $res));
var_dump($res);

echo "\nTest string with badly formed % numbers\n";
$str = "first=%41&second=%a&third=%b";
var_dump(parse_str($str, inout $res));
var_dump($res);

echo "\nTest string with non-binary safe name\n";
$str = "arr.test[1]=sid&arr test[4][two]=fred";
var_dump(parse_str($str, inout $res));
var_dump($res);
echo "===DONE===\n";
}

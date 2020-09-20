<?hh
/* Prototype  : string json_encode  ( mixed $value  [, int $options=0  ] )
 * Description: Returns the JSON representation of a value
 * Source code: ext/json/php_json.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing json_encode() : error conditions ***\n";

echo "\n-- Testing json_encode() function with no arguments --\n";
try { var_dump( json_encode() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing json_encode() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( json_encode("abc", 0, $extra_arg) );
echo "===Done===";
}

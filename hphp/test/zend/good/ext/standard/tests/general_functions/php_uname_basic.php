<?hh
/* Prototype: string php_uname  ([ string $mode  ] )
 * Description:  Returns information about the operating system PHP is running on
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing php_uname() - basic test\n";

var_dump(php_uname());

echo "\n-- Try all the defined mode's --\n";

var_dump(php_uname('a'));
var_dump(php_uname('s'));
var_dump(php_uname('n'));
var_dump(php_uname('r'));
var_dump(php_uname('v'));
var_dump(php_uname('m'));

echo "===DONE===\n";
}

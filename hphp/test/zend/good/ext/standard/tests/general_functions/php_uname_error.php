<?hh
/* Prototype: string php_uname  ([ string $mode  ] )
 * Description:  Returns information about the operating system PHP is running on */
class barClass {}
<<__EntryPoint>> function main(): void {
echo "*** Testing php_uname() - error test\n";

echo "\n-- Testing php_uname() function with more than expected no. of arguments --\n";
try { var_dump( php_uname('a', true) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing php_uname() function with invalid mode --\n";
// am invalid mode shoudl result in same o/p as mode 'a'
var_dump( php_uname('z') == php_uname('z') );

echo "===DONE===\n";
}

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

$fp = fopen(__FILE__, "r");




echo "\n-- Testing php_uname() function with invalid argument types --\n";
try { var_dump(php_uname(vec[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(php_uname(dict['color' => 'red', 'item' => 'pen'])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(php_uname(new barClass())); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(php_uname($fp)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

fclose($fp);
echo "===DONE===\n";
}

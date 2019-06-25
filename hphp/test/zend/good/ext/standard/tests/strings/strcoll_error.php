<?hh
/* Prototype: int strcoll  ( string $str1  , string $str2  )
   Description: Locale based string comparison
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strcoll() : error conditions ***\n";

echo "\n-- Testing strcoll() function with no arguments --\n";
try { var_dump( strcoll() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( strcoll("") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing strcoll() function with one argument --\n";
try { var_dump( strcoll("Hello World") );   } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing strcoll() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( strcoll("Hello World",  "World", $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===Done===";
}

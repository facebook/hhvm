<?hh
<<__EntryPoint>> function main(): void {
echo "*** Test by calling method or function with incorrect numbers of arguments ***\n";

$a = darray['bar' => 1];  
try { var_dump(key_exists()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(key_exists('foo', $a, 'baz')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

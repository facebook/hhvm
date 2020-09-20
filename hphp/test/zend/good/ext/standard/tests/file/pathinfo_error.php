<?hh
/* Prototype: mixed pathinfo ( string $path [, int $options] );
   Description: Returns information about a file path
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing pathinfo() for error conditions ***\n";
/* unexpected no. of arguments */
try { var_dump( pathinfo() );  /* args < expected */ } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( pathinfo("/home/1.html", 1, 3) );  /* args > expected */ } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}

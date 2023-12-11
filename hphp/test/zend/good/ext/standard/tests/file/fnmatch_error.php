<?hh
/* Prototype: bool fnmatch ( string $pattern, string $string [, int $flags] )
   Description: fnmatch() checks if the passed string would match 
     the given shell wildcard pattern. 
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions for fnmatch() ***";

/* Invalid arguments */
try { var_dump( fnmatch(vec[], vec[]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

$file_handle = fopen(__FILE__, "r");
try { var_dump( fnmatch($file_handle, $file_handle) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
fclose( $file_handle );

$std_obj = new stdClass();
try { var_dump( fnmatch($std_obj, $std_obj) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


/* No.of arguments less than expected */
try { var_dump( fnmatch("match.txt") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( fnmatch("") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* No.of arguments greater than expected */
try { var_dump( fnmatch("match.txt", "match.txt", TRUE, 100) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n*** Done ***\n";
}

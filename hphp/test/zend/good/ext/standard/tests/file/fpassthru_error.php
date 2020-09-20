<?hh
/*
 * Prototype: int fpassthru ( resource $handle );
 * Description: Reads to EOF on the given file pointer from the current position
 *  and writes the results to the output buffer.
*/
<<__EntryPoint>> function main(): void {
echo "*** Test error conditions of fpassthru() function ***\n";

/* Non-existing file resource */
$no_file = fopen("/no/such/file", "r");
try { var_dump( fpassthru($no_file) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* No.of args less than expected */
try { var_dump( fpassthru() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* No.of args greaer than expected */
try { var_dump( fpassthru("", "") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n*** Done ***\n";
}

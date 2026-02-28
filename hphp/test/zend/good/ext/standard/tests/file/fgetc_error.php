<?hh
/*
 Prototype: string fgetc ( resource $handle );
 Description: Gets character from file pointer
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions ***\n";
// zero argument
echo "-- Testing fgetc() with zero argument --\n";
try { var_dump( fgetc() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// more than expected no. of args
echo "-- Testing fgetc() with more than expected number of arguments --\n";
$fp = fopen(__FILE__, "r");
try { var_dump( fgetc($fp, $fp) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
fclose($fp);

echo "Done\n";
}

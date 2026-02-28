<?hh
/*
 Prototype: array fgetcsv ( resource $handle [, int $length [, string $delimiter [, string $enclosure [, string $escape]]]] );
 Description: Gets line from file pointer and parse for CSV fields
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions ***\n";
// zero argument
echo "-- Testing fgetcsv() with zero argument --\n";
try { var_dump( fgetcsv() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// more than expected no. of args
echo "-- Testing fgetcsv() with more than expected number of arguments --\n";
$fp = fopen(__FILE__, "r");
$len = 1024;
$delim = ";";
$enclosure ="\"";
$escape = '"';
try { var_dump( fgetcsv($fp, $len, $delim, $enclosure, $escape, $fp) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
fclose($fp);

echo "Done\n";
}

<?php
/* Prototype: string dirname ( string $path );
   Description: Returns directory name component of path.
*/
echo "*** Testing error conditions ***\n";
// zero arguments 
try { var_dump( dirname() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// more than expected no. of arguments
try { var_dump( dirname("/var/tmp/bar.gz", ".gz") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
?>

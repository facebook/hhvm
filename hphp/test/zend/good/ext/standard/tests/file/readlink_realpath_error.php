<?php
/* Prototype: string readlink ( string $path );
   Description: Returns the target of a symbolic link

   Prototype: string realpath ( string $path );
   Description: Returns canonicalized absolute pathname
*/

$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);

echo "*** Testing readlink(): error conditions ***\n";
try { var_dump( readlink() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args < expected
try { var_dump( readlink(__FILE__, 2) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected

echo "\n*** Testing readlink() on a non-existent link ***\n";
var_dump( readlink($file_path."/readlink_error.tmp") );

echo "\n*** Testing readlink() on existing file ***\n";
var_dump( readlink(__FILE__) );

echo "\n*** Testing readlink() on existing directory ***\n";
var_dump( readlink($file_path) );

echo "*** Testing realpath(): error conditions ***\n";
try { var_dump( realpath() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args < expected
try { var_dump( realpath(1, 2) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected

echo "\n*** Testing realpath() on a non-existent file ***\n";
var_dump( realpath($file_path."/realpath_error.tmp") );

echo "Done\n";
?>

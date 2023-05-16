<?hh
/* Prototype: string readlink ( string $path );
   Description: Returns the target of a symbolic link

   Prototype: string realpath ( string $path );
   Description: Returns canonicalized absolute pathname
*/
<<__EntryPoint>> function main(): void {


echo "*** Testing readlink(): error conditions ***\n";
try { var_dump( readlink() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args < expected
try { var_dump( readlink(__FILE__, 2) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected

echo "\n*** Testing readlink() on a non-existent link ***\n";
var_dump( readlink(sys_get_temp_dir().'/'.'readlink_error.tmp')) ;

echo "\n*** Testing readlink() on existing file ***\n";
var_dump( readlink(__FILE__) );

echo "\n*** Testing readlink() on existing directory ***\n";
var_dump( readlink(__DIR__) );

echo "*** Testing realpath(): error conditions ***\n";
try { var_dump( realpath() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args < expected
try { var_dump( realpath(1, 2) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected

echo "\n*** Testing realpath() on a non-existent file ***\n";
var_dump( realpath(sys_get_temp_dir().'/'.'realpath_error.tmp')) ;

echo "Done\n";
}

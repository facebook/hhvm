<?hh
/* Prototype: array lstat ( string $filename );
   Description: Gives information about a file or symbolic link

   Prototype: array stat ( string $filename );
   Description: Gives information about a file
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing lstat() for error conditions ***\n";
$file_path = dirname(__FILE__);
try { var_dump( lstat() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args < expected
try { var_dump( lstat(__FILE__, 2) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected
var_dump( lstat("$file_path/temp.tmp") ); // non existing file
var_dump( lstat('22') ); // scalar looking string
$arr = vec[__FILE__];
try { var_dump( lstat($arr) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // array argument

echo "\n*** Testing stat() for error conditions ***\n";
try { var_dump( stat() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args < expected
try { var_dump( stat(__FILE__, 2) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // file, args > expected
try { var_dump( stat(dirname(__FILE__), 2) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } //dir, args > expected

var_dump( stat("$file_path/temp.tmp") ); // non existing file
var_dump( stat("$file_path/temp/") ); // non existing dir
var_dump( stat('22') ); // scalar looking argument
try { var_dump( stat($arr) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // array argument

echo "Done\n";
}

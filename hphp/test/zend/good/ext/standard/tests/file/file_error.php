<?hh
/*
   Prototype: array file ( string filename [,int use-include_path [,resource context]] );
   Description: Reads entire file into an array
                Returns the  file in an array */
<<__EntryPoint>> function main(): void {
$filename = sys_get_temp_dir().'/'.'file.tmp';
fclose(fopen($filename, "w"));
echo "\n*** Testing error conditions ***";
try { var_dump( file() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // Zero No. of args


try { var_dump( file($filename, $filename, $filename, $filename) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // more than expected number of arguments

try { var_dump( file($filename, "INCORRECT_FLAG", NULL) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } //  Incorrect flag
var_dump( file($filename, 10, NULL) );  //  Incorrect flag

var_dump( file("temp.tmp") );  // non existing filename

echo "\n--- Done ---";

unlink($filename);
}

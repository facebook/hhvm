<?hh
/* 
 Prototype: array fgetcsv ( resource $handle [, int $length [, string $delimiter [, string $enclosure]]] );
 Description: Gets line from file pointer and parse for CSV fields
*/

/* Testing fgetcsv() to read from an empty file */
<<__EntryPoint>> function main(): void {
echo "*** Testing fgetcsv() : reading from file which is having zero content ***\n";

// try reading from file which is having zero content
// create the file and then open in read mode and try reading 
$filename = sys_get_temp_dir().'/'.'fgetcsv_variation23.tmp';
$fp = fopen ($filename, "w");
fclose($fp);
$fp = fopen ($filename, "r");
if (!$fp) {
  echo "Error: failed to create file $filename!\n";
  exit();
}
var_dump( fgetcsv($fp) );
var_dump( ftell($fp) );
var_dump( fgetcsv($fp, 1024) );
var_dump( ftell($fp) );
var_dump( fgetcsv($fp, 1024, "+" ) );
var_dump( ftell($fp) );
var_dump( fgetcsv($fp, 1024, "+", "%") );
var_dump( ftell($fp) );

// close and delete the file
fclose($fp);
unlink($filename);
echo "Done\n";
}

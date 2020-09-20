<?hh
/* Prototype: string readlink ( string $path );
   Description: Returns the target of a symbolic link

   Prototype: string realpath ( string $path );
   Description: Returns canonicalized absolute pathname
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing readlink() and realpath() : usage variations ***\n";

echo "\n*** Testing readlink() and realpath() with linkname as empty string, NULL and single space ***\n";
$link_string = varray [
  /* linkname as spaces */
  " ",
  ' ',

  /* empty linkname */
  "",
  '',
  NULL,
  null
 ];
for($loop_counter = 0; $loop_counter < count($link_string); $loop_counter++) {
  echo "-- Iteration";
  echo $loop_counter + 1;
  echo " --\n";

  var_dump( readlink((string)$link_string[$loop_counter]) );
  var_dump( realpath((string)$link_string[$loop_counter]) );
}

echo "Done\n";
}

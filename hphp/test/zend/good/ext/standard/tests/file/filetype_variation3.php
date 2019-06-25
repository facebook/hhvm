<?hh
/*
 * Prototype: string filetype ( string $filename );
 * Description: Returns the type of the file. Possible values are fifo, char,
 *              dir, block, link, file, and unknown.
*/
<<__EntryPoint>> function main(): void {
echo "-- Checking for block --\n";
print( filetype("/dev/ram0") )."\n";
echo "===DONE===\n";
}

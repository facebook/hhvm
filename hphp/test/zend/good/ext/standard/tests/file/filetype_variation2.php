<?hh
/*
 * Prototype: string filetype ( string $filename );
 * Description: Returns the type of the file. Possible values are fifo, char,
             dir, block, link, file, and unknown.
*/
<<__EntryPoint>> function main(): void {
echo "-- Checking for char --\n";
print( filetype("/dev/console") )."\n";
echo "===DONE===\n";
}

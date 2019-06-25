<?hh
/*
 * Prototype: string filetype ( string $filename );
 * Description: Returns the type of the file. Possible values are fifo, char,
   dir, block, link, file, and unknown.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing filetype() with files and dirs ***\n";

print( filetype(__FILE__) )."\n";
print( filetype(".") )."\n";

echo "*** Done ***\n";
}

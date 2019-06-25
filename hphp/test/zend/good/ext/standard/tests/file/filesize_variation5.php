<?hh
/*
 * Prototype   : int filesize ( string $filename );
 * Description : Returns the size of the file in bytes, or FALSE
 *               (and generates an error of level E_WARNING) in case of an error.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing filesize(): usage variations ***\n";

/* "", " " */
var_dump( filesize('') );
var_dump( filesize(' ') );
var_dump( filesize('|') );
echo "*** Done ***\n";
}

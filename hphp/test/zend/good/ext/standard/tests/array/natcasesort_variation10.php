<?hh
/* Prototype  : bool natcasesort(array &$array_arg)
 * Description: Sort an array using case-insensitive natural sort
 * Source code: ext/standard/array.c
 */

/*
 * Check position of internal array pointer after calling natcasesort()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing natcasesort() : usage variations ***\n";

$array_arg = vec['img13', 'img20', 'img2', 'img1'];

echo "\n-- Call natcasesort() --\n";
var_dump(natcasesort(inout $array_arg));
var_dump($array_arg);

echo "Done";
}

<?hh
/* Prototype  : proto array getimagesize(string imagefile [, array info])
 * Description: Get the size of an image as 4-element array
 * Source code: ext/standard/image.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing getimagesize() : xbm format ***\n";
$arr = null;
var_dump(getimagesize(dirname(__FILE__) . "/75x50.xbm", inout $arr));
var_dump($arr);

echo "===DONE===\n";
}

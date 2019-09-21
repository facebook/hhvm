<?hh
/* Prototype  : array getimagesize(string imagefile [, array info])
 * Description: Get the size of an image as 4-element array
 * Source code: ext/standard/image.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing getimagesize() : variation ***\n";

$info = null;
var_dump( getimagesize(dirname(__FILE__)."/test13pix.swf", inout $info) );
var_dump( $info );
echo "===DONE===\n";
}

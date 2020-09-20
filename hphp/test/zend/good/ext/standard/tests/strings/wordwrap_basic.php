<?hh
/* Prototype  : string wordwrap ( string $str [, int $width [, string $break [, bool $cut]]] )
 * Description: Wraps buffer to selected number of characters using string break char
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing wordwrap() : basic functionality ***\n";

// Initialize all required variables
$str = "The quick brown foooooooooox jummmmmmmmmmmmped over the lazzzzzzzzzzzy doooooooooooooooooooooog.";
$width = 80;
$break = '<br />\n';

// Calling wordwrap() with default arguments
var_dump( wordwrap($str) );

// Calling wordwrap() with all possible optional arguments
// with $width arg
var_dump( wordwrap($str, $width) );
// with $break arg
var_dump( wordwrap($str, $width, $break) );

// Calling wordwrap() with all arguments
// $cut as true
$width = 10;
$cut = true;
var_dump( wordwrap($str, $width, $break, $cut) );

// $cut as false
$width = 10;
$cut = false;
var_dump( wordwrap($str, $width, $break, $cut) );
echo "Done\n";
}

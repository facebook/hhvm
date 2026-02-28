<?hh
/* Prototype  : proto array getimagesize(string imagefile [, array info])
 * Description: Get the size of an image as 4-element array
 * Source code: ext/standard/image.c
 * Alias to functions:
 */

/*
 * Load APP info from jpeg  */
<<__EntryPoint>> function main(): void {
$arr = dict[];
$arr['this'] = "will";
$arr['all'] = "be destroyed!";
$arr['APP1'] = "and this too";

getimagesize(dirname(__FILE__)."/testAPP.jpg", inout $arr);

foreach ($arr as $key => $value) {
    echo "$key - length: ". strlen($value) ."; md5: " . md5($value) .  "\n" ;
}

echo "===DONE===\n";
}

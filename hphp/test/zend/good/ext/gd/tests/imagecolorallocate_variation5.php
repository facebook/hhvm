<?hh
/* Prototype  : int imagecolorallocate(resource im, int red, int green, int blue)
 * Description:  Allocate a color for an image
 * Source code: ext/gd/gd.c  */
<<__EntryPoint>> function main(): void {
echo "*** Testing imagecolorallocate() : usage variations ***\n";

$im = imagecreatetruecolor(200, 200);
$red = 10;
$green = 10;
$blue = 10;

$values = dict[
      // octal integer data
      "Octal 000" => 000,
      "Octal 012" => 012,
      "Octal -012" => -012,
      "Octal 0377" => 0377,

      // hexa-decimal integer data
      "Hexa-decimal 0x0" => 0x0,
      "Hexa-decimal 0xA" => 0xA,
      "Hexa-decimal -0xA" => -0xA,
      "Hexa-decimal 0xFF" => 0xFF,
];

// loop through each element of the array for blue
foreach($values as $key => $value) {
      echo "\n--$key--\n";
      var_dump( imagecolorallocate($im, $value, $green, $blue) );
      var_dump( imagecolorallocate($im, $red, $value, $blue) );
      var_dump( imagecolorallocate($im, $red, $green, $value) );
};
echo "===DONE===\n";
}

<?php
/* Prototype  : int imagecolorallocate(resource im, int red, int green, int blue)
 * Description:  Allocate a color for an image
 * Source code: ext/gd/gd.c
 */
echo "*** Testing imagecolorallocate() : usage variations ***\n";

$values = array(
      //Decimal integera data  
      "Decimal 256" => 256,
      
      // octal integer data
      "Octal 0400" => 0400,
     
      // hexa-decimal integer data
      "Hexa-decimal 0x100" => 0x100
);

// loop through each element of the array for blue
foreach($values as $key => $value) {
      echo "\n--$key--\n";
      //Need to be created every time to get expected return value
      $im_palette = imagecreate(200, 200);
      $im_true_color = imagecreatetruecolor(200, 200);
      var_dump( imagecolorallocate($im_palette, $value, $value, $value) );
      var_dump( imagecolorallocate($im_true_color, $value, $value, $value) );
};
?>
===DONE===
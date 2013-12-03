<?php
// Create a blank image and add some text
$im = imagecreatetruecolor(120, 20);
$text_color = imagecolorallocate($im, 255, 255, 255);
imagestring($im, 1, 5, 5,  'A Simple Text String', $text_color);

$file = dirname(__FILE__) .'/simpletext.png';

// Save the image as 'simpletext.png'
imagepng($im, $file);

// Free up memory
imagedestroy($im);

png2wbmp($file, '', 20, 120, 8);
png2wbmp($file, null, 20, 120, 8);
png2wbmp($file, false, 20, 120, 8);
?>
<?php
unlink(dirname(__FILE__) .'/simpletext.png');
?>

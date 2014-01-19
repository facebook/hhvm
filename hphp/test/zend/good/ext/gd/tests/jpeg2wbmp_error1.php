<?php
// Create a blank image and add some text
$im = imagecreatetruecolor(120, 20);
$text_color = imagecolorallocate($im, 255, 255, 255);
imagestring($im, 1, 5, 5,  'A Simple Text String', $text_color);

$file = dirname(__FILE__) .'/simpletext.jpg';
$file2 = dirname(__FILE__) .'/simpletext.wbmp';

// Save the image as 'simpletext.jpg'
imagejpeg($im, $file);

// Free up memory
imagedestroy($im);

jpeg2wbmp($file, $file2, 20, 120, 9);
jpeg2wbmp($file, $file2, 20, 120, -1);
?>
<?php
unlink(dirname(__FILE__) .'/simpletext.jpg');
unlink(dirname(__FILE__) .'/simpletext.wbmp');
?>

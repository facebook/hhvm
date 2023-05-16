<?hh
<<__EntryPoint>> function main(): void {
// Create a blank image and add some text
$im = imagecreatetruecolor(120, 20);
$text_color = imagecolorallocate($im, 255, 255, 255);
imagestring($im, 1, 5, 5,  'A Simple Text String', $text_color);

$file = sys_get_temp_dir().'/'.'simpletext.jpg';

// Save the image as 'simpletext.jpg'
imagejpeg($im, $file);

// Free up memory
imagedestroy($im);

jpeg2wbmp($file, '', 20, 120, 8);

unlink($file);
}

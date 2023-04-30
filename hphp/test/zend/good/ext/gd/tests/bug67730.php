<?hh
<<__EntryPoint>>
function entrypoint_bug67730(): void {
  // Create a blank image and add some text
  $im = imagecreatetruecolor(120, 20);
  $text_color = imagecolorallocate($im, 233, 14, 91);
  imagestring($im, 1, 5, 5,  'A Simple Text String', $text_color);

  // Save the image as 'simpletext.jpg'
  imagejpeg($im, sys_get_temp_dir().'/foo' . chr(0) . 'bar');

  // Free up memory
  imagedestroy($im);
}

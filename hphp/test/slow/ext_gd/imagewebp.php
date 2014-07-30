<?php
$filename= 'php.webp';

// Create a blank image and add some text
$im = imagecreatetruecolor(380, 60);
$text_color = imagecolorallocate($im, 233, 66, 91);

imagestring($im, 2, 9, 9,  'I\'ve got 99 problems but WebP ain\'t one.', $text_color);

// Save the image
imagewebp($im, $filename);

imagedestroy($im);

//We could check the MD5-hash here but maybe libvpx will get some
//improvements in the future that would change the output file.
echo filesize($filename)."\n";
$fp = fopen($filename, 'r');

$data = fread($fp, 14);   // read 14 Bytes from the WebP-File, that contains the header
echo preg_replace('/[[:^print:]]/', '?', $data); // remove non-ASCII chars

fclose($fp);
unlink($filename);

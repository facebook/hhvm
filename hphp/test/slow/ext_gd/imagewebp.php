<?php
$filename= './php.webp';

// Create a blank image and add some text
$im = imagecreatetruecolor(380, 60);
$text_color = imagecolorallocate($im, 233, 66, 91);

imagestring($im, 2, 9, 9,
            'I\'ve got 99 problems but WebP ain\'t one.', $text_color);

// Save the image
imagewebp($im, $filename, 99);

imagedestroy($im);

// We could check the MD5-hash here but maybe libvpx will get some
// improvements in the future that would change the output file.
// We just check if the file is a valid WebP-File and has
// approximately the right size.
echo floor(filesize($filename)/100)*100, "\n";
$fp = fopen($filename, 'r');

// read 14 Bytes from the WebP-File, that contains the header
$data = fread($fp, 14);

// remove non-ASCII chars
echo preg_replace('/[[:^print:]]/', '?', $data);

fclose($fp);
unlink($filename);

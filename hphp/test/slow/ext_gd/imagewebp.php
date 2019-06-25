<?hh <<__EntryPoint>> function main(): void {
$filename= './php.webp';

// Create a blank image and add some text
$im = imagecreatetruecolor(380, 60);
$text_color = imagecolorallocate($im, 233, 66, 91);

imagestring($im, 2, 9, 9,
            'I\'ve got 99 problems but WebP ain\'t one.', $text_color);

// Save the image
imagewebp($im, $filename);

imagedestroy($im);

$size = filesize($filename) - 7; //Subtract FourCC + length header
$fp = fopen($filename, 'r');

// read 14 Bytes from the WebP-File, that contains the header
$header = fread($fp, 14);
$header = unpack("A4fourcc/L1length/A4chunkheader", $header);

if ($header["fourcc"] != "RIFF") echo "Invalid FourCC in created image file\n";
if ($header["length"] != $size) echo "The length specified in the image header is different from the actual size: (${header['length']} != ${size})\n";

fclose($fp);
unlink($filename);
echo "OK!\n";
}

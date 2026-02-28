<?hh

<<__EntryPoint>>
function main(): void {
$jpeg_file = sys_get_temp_dir().'/'.'imagick_test.jpg';
$png_file = sys_get_temp_dir().'/'.'imagick_test.png';

$im = new Imagick ('magick:rose');
$im->writeImage ($jpeg_file);
$im->clear ();

// This is the problematic case, setImageFormat doesn't really
// affect writeImageFile.
// So in this case we want to write PNG but file should come out
// as JPEG
$fp = fopen ($png_file, "w+");
$im->readImage ($jpeg_file);
$im->setImageFormat ('png');
$im->writeImageFile ($fp);
$im->clear ();
fclose ($fp);

// Output the format
$identify = new Imagick ($png_file);
echo $identify->getImageFormat () . PHP_EOL;

// Lets try again, setting the filename rather than format
// This should cause PNG image to be written
$fp = fopen ($png_file, "w+");
$im->readImage ($jpeg_file);
$im->setImageFilename ('png:');
$im->writeImageFile ($fp);
$im->clear ();
fclose ($fp);

// If all goes according to plan, on second time we should get PNG
$identify = new Imagick ($png_file);
echo $identify->getImageFormat () . PHP_EOL;

// Lastly, test the newly added format parameter
$fp = fopen ($png_file, "w+");
$im->readImage ($jpeg_file);
$im->writeImageFile ($fp, 'png');
$im->clear ();
fclose ($fp);

// If all goes according to plan, on second time we should get PNG
$identify = new Imagick ($png_file);
echo $identify->getImageFormat () . PHP_EOL;

unlink ($png_file);
unlink ($jpeg_file);

echo 'done' . PHP_EOL;
}

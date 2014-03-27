<?php

define ("JPEG_FILE", __DIR__."/imagick_test.jpg");
define ("PNG_FILE", __DIR__."/imagick_test.png");

$im = new imagick ('magick:rose');
$im->writeImage (JPEG_FILE);
$im->clear ();

// This is the problematic case, setImageFormat doesn't really
// affect writeImageFile.
// So in this case we want to write PNG but file should come out
// as JPEG
$fp = fopen (PNG_FILE, "w+");
$im->readImage (JPEG_FILE);
$im->setImageFormat ('png');
$im->writeImageFile ($fp);
$im->clear ();
fclose ($fp);

// Output the format
$identify = new Imagick (PNG_FILE);
echo $identify->getImageFormat () . PHP_EOL;

// Lets try again, setting the filename rather than format
// This should cause PNG image to be written
$fp = fopen (PNG_FILE, "w+");
$im->readImage (JPEG_FILE);
$im->setImageFilename ('png:');
$im->writeImageFile ($fp);
$im->clear ();
fclose ($fp);

// If all goes according to plan, on second time we should get PNG
$identify = new Imagick (PNG_FILE);
echo $identify->getImageFormat () . PHP_EOL;

// Lastly, test the newly added format parameter
$fp = fopen (PNG_FILE, "w+");
$im->readImage (JPEG_FILE);
$im->writeImageFile ($fp, 'png');
$im->clear ();
fclose ($fp);

// If all goes according to plan, on second time we should get PNG
$identify = new Imagick (PNG_FILE);
echo $identify->getImageFormat () . PHP_EOL;

unlink (PNG_FILE);
unlink (JPEG_FILE);

echo 'done' . PHP_EOL;
?>

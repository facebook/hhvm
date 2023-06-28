<?hh


// Create an image instance
<<__EntryPoint>>
function main_1786() :mixed{
$im = imagecreatefromgif(__DIR__.'/images/php.gif');
// Enable interlancing
imageinterlace($im, 1);
// Save the interfaced image
imagegif($im);
imagedestroy($im);
}

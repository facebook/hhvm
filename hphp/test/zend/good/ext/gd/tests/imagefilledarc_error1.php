<?hh
<<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(100, 100);

$white = imagecolorallocate($image, 0xFF, 0xFF, 0xFF);

//create an arc and fill it with white color    
try { imagefilledarc($image, 50, 50, 30, 30, 0, 90, $white); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

ob_start();
imagepng($image);
$img = ob_get_contents();
ob_end_clean();

echo md5(base64_encode($img));
}

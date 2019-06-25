<?hh
<<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(180, 30);
$white = imagecolorallocate($image, 255, 255, 255);
try { $result = imagecolordeallocate('image', $white); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

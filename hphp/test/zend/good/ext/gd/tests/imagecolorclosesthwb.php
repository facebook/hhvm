<?hh <<__EntryPoint>> function main(): void {
$im = imagecreatefrompng(dirname(__FILE__).'/test.png');

var_dump(imagecolorclosesthwb($im, 255, 50, 0));

try { var_dump(imagecolorclosesthwb(NULL)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

imagedestroy($im);
}

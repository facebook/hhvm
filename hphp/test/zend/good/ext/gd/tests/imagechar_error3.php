<?hh <<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(180, 30);
try { $result = imagechar($image, 'string', 5, 5, 'C', 1); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

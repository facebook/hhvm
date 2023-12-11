<?hh <<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(50, 50);
$resource = tmpfile();

try { imagetruecolortopalette($image, $resource, 2); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { imagetruecolortopalette($image, vec[], 2); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

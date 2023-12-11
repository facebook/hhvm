<?hh <<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(50, 50);
$resource = tmpfile();

try { imagetruecolortopalette($image, true, 'string'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { imagetruecolortopalette($image, true, $resource); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { imagetruecolortopalette($image, true, vec[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { imagetruecolortopalette($image, true, null); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

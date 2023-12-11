<?hh <<__EntryPoint>> function main(): void {
$img = imagecreatetruecolor(200, 200);

try { imagecolorallocatealpha($img, 255, 255, 255, 'string-non-numeric'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { imagecolorallocatealpha($img, 255, 255, 255, vec[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { imagecolorallocatealpha($img, 255, 255, 255, tmpfile()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

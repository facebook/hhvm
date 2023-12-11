<?hh <<__EntryPoint>> function main(): void {
$img = imagecreatetruecolor(200, 200);

try { imagecolorallocatealpha($img, 'string-non-numeric', 255, 255, 50); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { imagecolorallocatealpha($img, vec[], 255, 255, 50); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { imagecolorallocatealpha($img, tmpfile(), 255, 255, 50); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

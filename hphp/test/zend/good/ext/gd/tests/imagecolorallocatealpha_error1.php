<?hh <<__EntryPoint>> function main(): void {
$resource = tmpfile();

imagecolorallocatealpha($resource, 255, 255, 255, 50);
try { imagecolorallocatealpha('string', 255, 255, 255, 50); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { imagecolorallocatealpha(vec[], 255, 255, 255, 50); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { imagecolorallocatealpha(null, 255, 255, 255, 50); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

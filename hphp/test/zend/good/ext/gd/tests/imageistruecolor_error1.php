<?hh <<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(180, 30);
$resource = tmpfile();

try { imageistruecolor('string'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
imageistruecolor($resource);
try { imageistruecolor(vec[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

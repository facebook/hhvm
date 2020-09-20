<?hh <<__EntryPoint>> function main(): void {
try { $image = imagecreatetruecolor('s', 30); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { $image = imagecreatetruecolor(30, 's'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

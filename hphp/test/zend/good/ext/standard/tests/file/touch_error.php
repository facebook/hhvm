<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(touch()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(touch(1, 2, 3, 4)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(touch("/no/such/file/or/directory"));
}

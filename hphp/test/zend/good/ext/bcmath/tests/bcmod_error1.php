<?hh <<__EntryPoint>> function main(): void {
try { echo bcmod('1', '2', '3'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

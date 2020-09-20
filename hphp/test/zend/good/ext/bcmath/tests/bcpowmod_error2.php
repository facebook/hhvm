<?hh <<__EntryPoint>> function main(): void {
try { echo bcpowmod('1', '2'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

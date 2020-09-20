<?hh <<__EntryPoint>> function main(): void {
try { echo bcpowmod('1', '2', '3', '4', '5'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

<?hh <<__EntryPoint>> function main(): void {
try { var_dump($biggest_int = getrandmax(true)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

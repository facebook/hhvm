<?hh
<<__EntryPoint>> function main(): void {
try { $nano = time_nanosleep('A', 100000); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

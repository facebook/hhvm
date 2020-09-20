<?hh
<<__EntryPoint>> function main(): void {
try { $nano = time_nanosleep(2, 'B'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

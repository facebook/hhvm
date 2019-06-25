<?hh <<__EntryPoint>> function main(): void {
try { var_dump(time_sleep_until()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

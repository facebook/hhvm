<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(SQLite3::version('dummy')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

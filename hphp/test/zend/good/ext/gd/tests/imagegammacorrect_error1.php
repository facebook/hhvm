<?hh
<<__EntryPoint>> function main(): void {
try { $gamma = imagegammacorrect('string', 1, 5); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

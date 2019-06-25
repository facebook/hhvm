<?hh
<<__EntryPoint>> function main(): void {
try { $result = imagestring('string', 1, 5, 5, 'String', 1); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

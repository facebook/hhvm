<?hh
<<__EntryPoint>> function main() {
try { $result = imagechar('string', 1, 5, 5, 'C', 1); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

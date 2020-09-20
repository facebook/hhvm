<?hh <<__EntryPoint>> function main(): void {
$dom = new DOMDocument('1.0');
try { $dom->validate(true); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

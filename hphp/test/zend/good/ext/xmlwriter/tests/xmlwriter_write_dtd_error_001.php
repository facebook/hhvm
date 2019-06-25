<?hh <<__EntryPoint>> function main(): void {
$xmlwriter = xmlwriter_open_memory();
try { var_dump(xmlwriter_write_dtd($xmlwriter)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

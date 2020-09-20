<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(sys_getloadavg("")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(sys_getloadavg());

echo "Done\n";
}

<?hh <<__EntryPoint>> function main(): void {
try { $sock1 = socket_create_listen(varray[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { $sock2 = socket_create_listen(31337, varray[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

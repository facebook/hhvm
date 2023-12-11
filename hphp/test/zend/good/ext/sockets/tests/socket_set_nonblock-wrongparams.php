<?hh <<__EntryPoint>> function main(): void {
try { $socket = socket_set_nonblock(vec[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

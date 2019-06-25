<?hh <<__EntryPoint>> function main(): void {
try { var_dump(socket_listen(null)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
$socket = socket_create(AF_UNIX, SOCK_STREAM, 0);
var_dump(socket_listen($socket));
}

<?hh

<<__EntryPoint>>
function entrypoint_socket_connect_error(): void {
    // Test with no arguments
    try { $server = socket_create(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

    // Test with less arguments than required
    try { $server = socket_create(SOCK_STREAM, getprotobyname('tcp')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

    // Test with non integer parameters
    try { $server = socket_create(vec[], 1, 1); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

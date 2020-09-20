<?hh <<__EntryPoint>> function main(): void {
try { var_dump(socket_create_pair(AF_INET, null, null)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

$domain = 'unknown';
$sockets = null;
try { var_dump(socket_create_pair($domain, SOCK_STREAM, 0, inout $sockets)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

try { var_dump(socket_create_pair(AF_INET, null, null, inout $sockets)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

try { var_dump(socket_create_pair(31337, null, null, inout $sockets)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

try { var_dump(socket_create_pair(AF_INET, 31337, 0, inout $sockets)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

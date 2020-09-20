<?hh <<__EntryPoint>> function main(): void {
try { $s_s = socket_strerror(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
for ($i=0;$i<=132;$i++) {
    var_dump(socket_strerror($i));
}
}

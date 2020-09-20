<?hh <<__EntryPoint>> function main(): void {
$rand = rand(1,999);
$s_c_l = null;
try { $s_c_l = socket_create_listen(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump($s_c_l);
if (is_resource($s_c_l)) {
    try { @socket_close($s_c_l); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
}

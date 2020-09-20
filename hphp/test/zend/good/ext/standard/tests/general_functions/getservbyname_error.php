<?hh <<__EntryPoint>> function main(): void {
$service = "www";
$protocol = "tcp";
$extra_arg = 12;
try { var_dump(getservbyname($service, $protocol, $extra_arg ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(getservbyname($service)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

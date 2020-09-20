<?hh <<__EntryPoint>> function main(): void {
$port = 80;
$protocol = "tcp";
$extra_arg = 12;
try { var_dump(getservbyport( $port, $protocol, $extra_arg ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(getservbyport($port)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

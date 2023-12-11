<?hh <<__EntryPoint>> function main(): void {
try { var_dump( posix_getsid() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( posix_getsid(vec[]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump( posix_getsid(-1) );
echo "===DONE===\n";
}

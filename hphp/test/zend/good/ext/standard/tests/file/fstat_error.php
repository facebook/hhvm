<?hh <<__EntryPoint>> function main(): void {
$fp = fopen (__FILE__, 'r');
$extra_arg = 'nothing';

try { var_dump(fstat( $fp, $extra_arg ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(fstat()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

fclose($fp);

echo "===DONE===\n";
}

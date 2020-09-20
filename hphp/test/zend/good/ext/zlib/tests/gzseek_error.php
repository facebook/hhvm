<?hh <<__EntryPoint>> function main(): void {
$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r');
$offset = 1;
$whence = SEEK_SET;
$extra_arg = 'nothing';

try { var_dump(gzseek( $h, $offset, $whence, $extra_arg ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gzseek($h)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gzseek()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}

<?hh <<__EntryPoint>> function main(): void {
$filename = sys_get_temp_dir().'/'."gzwrite_error.txt.gz";
$h = gzopen($filename, 'w');
$str = "Here is the string to be written. ";
$length = 10;
$extra_arg = 'nothing';
try { var_dump(gzwrite($h, $str, $length, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gzwrite($h)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gzwrite()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

gzclose($h);
unlink($filename);

echo "===DONE===\n";
}

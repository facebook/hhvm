<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(gzfile()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gzfile("nonexistent_file_gzfile",1));
try { var_dump(gzfile(1,1,1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(gzfile(dirname(__FILE__)."/004.txt.gz"));
var_dump(gzfile(dirname(__FILE__)."/004.txt.gz", 1));

echo "Done\n";
}

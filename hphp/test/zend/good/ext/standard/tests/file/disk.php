<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(disk_free_space()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(disk_total_space()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(disk_free_space('-1'));
var_dump(disk_total_space('-1'));

var_dump(disk_free_space("/"));
var_dump(disk_total_space("/"));

var_dump(disk_free_space("/some/path/here"));
var_dump(disk_total_space("/some/path/here"));

echo "Done\n";
}

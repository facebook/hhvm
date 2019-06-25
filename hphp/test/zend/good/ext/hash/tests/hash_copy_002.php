<?hh
<<__EntryPoint>> function main(): void {
$r = hash_init("md5");
try { var_dump(hash_copy()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(hash_copy($r));
try { var_dump(hash_copy($r, $r)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}

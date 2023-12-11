<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(quoted_printable_encode()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(quoted_printable_encode(""));
var_dump(quoted_printable_encode("test"));
try { var_dump(quoted_printable_encode("test", "more")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

$a = vec["str"];
try { var_dump(quoted_printable_encode($a)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(quoted_printable_encode('1'));
var_dump(quoted_printable_encode(''));

echo "Done\n";
}

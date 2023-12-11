<?hh
<<__EntryPoint>> function main(): void {
$a = 1;
$b = vec[1];
$c = vec[1];
$d = vec[1];
$iv = null;

try { var_dump(openssl_seal($a, inout $b, inout $c, $d, '', inout $iv)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(openssl_seal($a, inout $a, inout $a, vec[], '', inout $iv)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(openssl_seal($c, inout $c, inout $c, 1, '', inout $iv)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(openssl_seal($b, inout $b, inout $b, "", '', inout $iv)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}

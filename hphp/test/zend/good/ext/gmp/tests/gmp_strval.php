<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(gmp_strval()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_strval(""));
try { var_dump(gmp_strval("", "")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_strval("", -1));
try { var_dump(gmp_strval(-1, "")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

$fp = fopen(__FILE__, "r");
var_dump(gmp_strval($fp));

$g = gmp_init("9765456");
var_dump(gmp_strval($g));
var_dump(gmp_strval($g, -1));
var_dump(gmp_strval($g, 100000));
var_dump(gmp_strval($g, 10));

$g = gmp_init("-3373333");
var_dump(gmp_strval($g));
var_dump(gmp_strval($g, -1));
var_dump(gmp_strval($g, 100000));
var_dump(gmp_strval($g, 10));

try { var_dump(gmp_strval(vec[1,2], vec[1,2])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_strval(new stdClass, new stdClass)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_strval(vec[1,2]));
var_dump(gmp_strval(new stdClass));

echo "Done\n";
}

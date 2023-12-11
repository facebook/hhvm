<?hh

const TEST_CONST = 1;
const TEST_CONST2 = "test";

<<__EntryPoint>> function main(): void {
try { var_dump(constant()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(constant("", "")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(constant(""));

try { var_dump(constant(vec[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(constant("TEST_CONST"));

var_dump(constant("TEST_CONST2"));

echo "Done\n";
}

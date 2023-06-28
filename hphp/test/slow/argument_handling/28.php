<?hh

function test($str) :mixed{
  return strlen($str);
}

<<__EntryPoint>>
function main_28() :mixed{
try { var_dump(strlen()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { test(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(strlen('test'));
var_dump(test('test'));

try { var_dump(strlen('test', 123)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(test('test', 123));
}

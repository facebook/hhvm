<?hh

<<__EntryPoint>>
function main_coerce(): mixed{
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });
  try { var_dump(Locale::lookup(new stdClass, 'foo')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(Locale::lookup(fopen(__FILE__, 'r'), 'foo')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

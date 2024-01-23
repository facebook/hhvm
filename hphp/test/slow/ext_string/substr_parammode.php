<?hh

<<__EntryPoint>>
function main_substr_parammode(): mixed {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });
  try { var_dump(substr(vec[], 0)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

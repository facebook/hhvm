<?hh
class A {}
<<__EntryPoint>>
function main(): void {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });
  var_dump(get_object_vars(new A));
  try { var_dump(get_object_vars(false)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(get_object_vars(true)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(get_object_vars('hello')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(get_object_vars(5)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

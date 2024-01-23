<?hh

function wrap($fn) :mixed{
  try {
    var_dump($fn());
  } catch(Exception $e) {
    var_dump(get_class($e), $e->getMessage());
  }
}

<<__EntryPoint>>
function main(): void {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });
  $map = Map { 42 => 'foo' };
  wrap(() ==> $map->get(false));
  wrap(() ==> $map->at(false));
}

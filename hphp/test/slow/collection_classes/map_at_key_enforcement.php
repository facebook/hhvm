<?hh

function wrap($fn) {
  try {
    var_dump($fn());
  } catch(Exception $e) {
    var_dump(get_class($e), $e->getMessage());
  }
}

<<__EntryPoint>>
function main(): void {
  $map = Map { 42 => 'foo' };
  wrap(() ==> $map->get(false));
  wrap(() ==> $map->at(false));
}

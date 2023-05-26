<?hh

module a.b;

function pp_exn(Exception $e) {
  return $e->getMessage() . " @ " . $e->getFile() . ":" . $e->getLine() . "\n";
}

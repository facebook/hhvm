<?hh

function gen() {
  $e = yield null;
  throw $e;
}

<<__EntryPoint>>
function main() {
  $g = gen();
  $g->next();
  try {
    $g->send(new Exception('boom'));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

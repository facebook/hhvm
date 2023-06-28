<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
  $e = yield null;
  throw $e;
}

<<__EntryPoint>>
function main() :mixed{
  $g = gen();
  $g->next();
  try {
    $g->send(new Exception('boom'));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

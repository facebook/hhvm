<?hh

function squares_cubes() :AsyncGenerator<mixed,mixed,void>{
  $i = 0;
  for (;;) {
    $i++;
    yield $i*$i => $i*$i*$i;
  }
}


<<__EntryPoint>>
function main_2229() :mixed{
  $c = squares_cubes();
  $c->next();
  var_dump($c->key());
  var_dump($c->current());
  $c->next();
  var_dump($c->key());
  var_dump($c->current());
}

<?hh

function gen() {
  yield;
}
<<__EntryPoint>> function main(): void {
$gen = gen();
$gen->next();
$gen->next();
var_dump($gen->valid());
$gen->throw(new Exception('test'));
}
